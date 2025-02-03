#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameTimer.cpp
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TimerEvent {
public:
    using ExecutionFn = std::function<void()>;

public:
    TimerEvent() = default;
    ~TimerEvent() = default;

    TimerEvent(const TimerEvent& other);
    TimerEvent(TimerEvent&& other);
    TimerEvent& operator=(const TimerEvent& other);
    TimerEvent& operator=(TimerEvent&& other);

private:
    std::atomic_bool mExecuting{ };
    ExecutionFn mFunction{ };
};

class GameTimer { 
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

public:
    UINT32 GetFps() const;
    float GetDeltaTime() const;

    float Update();

private:
    float mDeltaTime{ };
    float mTimeScale{ };
    UINT32 mFps{ };

    Concurrency::concurrent_queue<TimerEvent> mEventQueue{ };
};