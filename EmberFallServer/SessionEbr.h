#pragma once

#include <queue>
#include <atomic>
#include <thread>
#include "GameSession.h"
//#include "utils.h"

//#define DEBUG_PRINT

using ThreadIdType = int32_t;
using EpochNumType = uint64_t;
using EpochCounter = std::atomic<EpochNumType>;

inline const size_t HARDWARE_CONCURRENCY = std::thread::hardware_concurrency() * 2;
inline constexpr size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;

template <typename T>
inline constexpr size_t MIN_ARRAY_ALIGN_SIZE = CACHE_LINE_SIZE / sizeof(T);

template <typename T>
concept HasEpochCount = requires(T t) {
    t.mEpochCounter;
    { t.mEpochCounter } -> std::same_as<EpochNumType>;
};

class SessionEbr {
private:
    inline static constexpr size_t ALIGN_SIZE = MIN_ARRAY_ALIGN_SIZE<EpochCounter>;

public:
    SessionEbr() : mEpochArr{ std::make_unique<EpochCounter[]>(MAX_THREAD * ALIGN_SIZE) }, mEpochCounter{ 1 } {}
    ~SessionEbr() { ClearThreadEpoch(); }

public:
    EpochNumType GetCurrEpochCount() const {
        return mEpochCounter;
    }

    void BeginEpoch(const ThreadIdType id) {
        auto epoch = mEpochCounter.fetch_add(1);
        mEpochArr[id * ALIGN_SIZE] = epoch;
    }

    void EndEpoch(const ThreadIdType id) {
        mEpochArr[id * ALIGN_SIZE] = 0;
    }

    void ClearThreadEpoch() {
        while (false == mFreeQueue.empty()) {
            auto ptr = mFreeQueue.front();
            mFreeQueue.pop();
            if (nullptr == ptr) {
                continue;
            }

            delete ptr;
        }

        mEpochCounter = 1;
    }

    void PushPointer(GameSession* ptr) {
        ptr->mEpochCounter = mEpochCounter.load();
        mFreeQueue.push(ptr);
    }

    template <typename ObjectType, typename... Args> requires
        std::derived_from<ObjectType, GameSession> and std::is_constructible_v<ObjectType, Args...>
        ObjectType* PopPointer(Args&&... args) {
        if (true == mFreeQueue.empty()) {
            return new ObjectType{ std::forward<Args>(args)... };
        }

        auto ptr = mFreeQueue.front();
        for (size_t threadId = 0; threadId < MAX_THREAD * ALIGN_SIZE; threadId += ALIGN_SIZE) {
            if (mEpochArr[threadId] != 0 && mEpochArr[threadId] < ptr->mEpochCounter) {
                return new ObjectType{ std::forward<Args>(args)... };
            }
        }

        mFreeQueue.pop();

#ifdef DEBUG_PRINT
        std::cout << "Reuse Session Pointer\n";
#endif

        std::destroy_at(ptr);
        //auto ret = cast_ptr<ObjectType>(ptr);
        return std::construct_at<ObjectType>(reinterpret_cast<ObjectType>(ptr), std::forward<Args>(args)...);
    }

public:
    inline static thread_local std::queue<GameSession*> mFreeQueue{ };
    inline static const size_t MAX_THREAD = HARDWARE_CONCURRENCY;

private:
    EpochCounter mEpochCounter{ };
    const std::unique_ptr<EpochCounter[]> mEpochArr;
};

class SessionEbrGuard {
public:
    SessionEbrGuard() = delete;
    SessionEbrGuard(const SessionEbrGuard&) = delete;
    SessionEbrGuard(SessionEbrGuard&&) = delete;
    SessionEbrGuard& operator=(const SessionEbrGuard&) = delete;
    SessionEbrGuard& operator=(SessionEbrGuard&&) = delete;

    explicit SessionEbrGuard(SessionEbr& ebr, const int32_t threadId) noexcept : mEbr{ ebr }, mThreadId{ threadId } {
        mEbr.BeginEpoch(mThreadId);
    }

    ~SessionEbrGuard() noexcept {
        mEbr.EndEpoch(mThreadId);
    }

private:
    const int32_t mThreadId;
    SessionEbr& mEbr;
};