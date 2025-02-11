#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameTimer.h
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TimerEvent {
public:
    using ExecutionFn = std::function<void()>;

public:
    TimerEvent();
    ~TimerEvent();

    TimerEvent(const TimerEvent& other);
    TimerEvent(TimerEvent&& other) noexcept;
    TimerEvent& operator=(const TimerEvent& other);
    TimerEvent& operator=(TimerEvent&& other) noexcept;

private:
    std::atomic_bool mExecuting{ };
    ExecutionFn mFunction{ };
};

class GameTimer { 
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePeriod = std::milli;
    using Duration = std::chrono::duration<float, TimePeriod>;
    using TimePoint = std::chrono::high_resolution_clock::time_point;

public:
    GameTimer();
    ~GameTimer();

public:
    UINT32 GetFps() const;
    float GetDeltaTime() const;

    void Sync(INT32 syncFrame=0);
    void Update();

private:
    float mSyncRatio{ };
    float mDeltaTime{ };
    float mTimeScale{ };
    float mFpsCounter{ };
    UINT32 mFps{ };

    TimePoint mPrevPoint{ };

    Concurrency::concurrent_queue<TimerEvent> mEventQueue{ };
};