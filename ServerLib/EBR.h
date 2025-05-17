#pragma once

#include <queue>

using EpochNumType = uint64_t;
using EpochCounter = std::atomic<EpochNumType>;

template <typename T>
inline constexpr size_t MIN_ARRAY_ALIGN_SIZE = CACHE_LINE_SIZE / sizeof(T);

template <typename T>
concept HasEpochCount = requires(T t) {
    t.mEpochCount;
    { t.mEpochCount } -> std::same_as<EpochNumType>;
};

template <typename T>
class EBR {
protected:
    inline static constexpr size_t ALIGN_SIZE = MIN_ARRAY_ALIGN_SIZE<std::atomic_uint64_t>;

public:
    EBR() : mEBRArray{ std::make_unique<EpochCounter[]>(MAX_THREAD * ALIGN_SIZE) }, mEpochCounter{ 1 } {}
    ~EBR() { }

public:
    void StartEpoch(const ThreadIdType id) {
        auto epoch = mEpochCounter.fetch_add(1);
        mEBRArray[id * ALIGN_SIZE] = epoch;
    }

    void EndEpoch(const ThreadIdType id) {
        mEBRArray[id * ALIGN_SIZE] = 0;
    }

    //
    void CleanupThreadLocalQueue() {
        while (false == mFreeQueue.empty()) {
            auto ptr = mFreeQueue.front();
            mFreeQueue.pop();
            if (nullptr == ptr) {
                continue;
            }

            std::destroy_at(ptr);
            delete ptr;
        }
    }

    void PushPointer(T* ptr) {
        ptr->mEpochCount = mEpochCounter.load();
        mFreeQueue.push(ptr);
    }

    template <typename... Args>
    T* PopPointer(Args&&... args) {
        if (true == mFreeQueue.empty()) {
            return new T{ std::forward<Args>(args)... };
        }

        auto ptr = mFreeQueue.front();
        for (size_t i = 0; i < MAX_THREAD * ALIGN_SIZE; i += ALIGN_SIZE) {
            if (0 != mEBRArray[i] and mEBRArray[i] < ptr->mEpochCount) {
                return new T{ std::forward<Args>(args)... };
            }
        }

        mFreeQueue.pop();

        std::destroy_at(ptr);
        new(ptr) T{ std::forward<Args>(args)... };
        return ptr;
    }

public:
    inline static thread_local std::queue<T*> mFreeQueue{ };
    inline static const size_t MAX_THREAD = HARDWARE_CONCURRENCY;

protected:
    EpochCounter mEpochCounter{ };
    const std::unique_ptr<EpochCounter[]> mEBRArray;
};

template <typename T>
class EBRGaurd {
public:
    explicit EBRGuard(EBR<T>& ebr) 
        : mEBR{ ebr } { 
        mEBR.StartEpoch();
    }

    ~EBRGuard() {
        mEBR.EndEpoch();
    }

private:
    EBR<T>& mEBR;
};