#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameTimer.h
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GameTimer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePeriod = std::milli;
    using Duration = std::chrono::duration<float, TimePeriod>;
    using TimePoint = Clock::time_point;
    using EventCallBack = std::function<void()>;

public:
    class TimerEvent {
    public:
        TimerEvent(EventCallBack&& callback, std::chrono::time_point<Clock> time);
        ~TimerEvent();

        TimerEvent(const TimerEvent& other);
        TimerEvent(TimerEvent&& other) noexcept;
        TimerEvent& operator=(const TimerEvent& other);
        TimerEvent& operator=(TimerEvent&& other) noexcept;

    public:
        bool operator<(const TimerEvent& right) const;

    private:
        EventCallBack mFunction{ };
        TimePoint mTimeRegistered{ };
        bool mLoop{ false };
    };

public:
    GameTimer();
    ~GameTimer();

public:
    UINT32 GetFps() const;
    float GetDeltaTime() const;
    float GetTimeFromStart() const;

    void Sync(INT32 syncFrame = 0);
    void Update();

public:
    static void PushTimerEvent(EventCallBack&& callback, Duration time);

private:
    inline static std::priority_queue<TimerEvent> mEventQueue{ };

private:
    float mSyncRatio{ };
    float mDeltaTime{ };
    float mTimeScale{ };
    float mFpsCounter{ };
    UINT32 mFps{ };

    TimePoint mPrevPoint{ };
    TimePoint mPointSinceStart{ };
};

class StaticTimer {
public:
    static UINT32 GetFps();
    static float GetDeltaTime();
    static float GetTimeFromStart();

    static void Update();

private:
    inline static GameTimer mTimer{ };
};

using TimerEvent = GameTimer::TimerEvent;