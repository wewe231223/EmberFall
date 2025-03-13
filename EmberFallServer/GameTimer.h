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
        TimerEvent(EventCallBack&& callback, TimePoint timeRegistered, Duration time, int32_t loopCount);
        ~TimerEvent();

        TimerEvent(const TimerEvent& other) = delete;
        TimerEvent& operator=(const TimerEvent& other) = delete;

        TimerEvent(TimerEvent&& other) noexcept;
        TimerEvent& operator=(TimerEvent&& other) noexcept;

    public:
        bool operator<(const TimerEvent& right) const;

    public:
        EventCallBack GetFunction() const;
        Duration GetDuration() const;
        int32_t GetLoopCount() const;
        bool IsTimeSinceResistered(std::chrono::time_point<Clock> time) const;

        void Excute() const;

    private:
        int32_t mLoopCount{ };
        TimePoint mTimeRegistered{ };
        Duration mDelay{ };
        EventCallBack mFunction{ };
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
    
private:
    void ProcessEvent();

public:
    void PushTimerEvent(EventCallBack&& callback, Duration time, int32_t loopCount);

private:
    std::priority_queue<TimerEvent> mEventQueue{ };

private:
    float mSyncRatio{ };
    float mDeltaTime{ };
    float mTimeScale{ };
    float mFpsCounter{ };
    UINT32 mFpsCount{ };
    UINT32 mFps{ };

    TimePoint mPrevPoint{ };
    TimePoint mPointSinceStart{ };
};

using TimerEvent = GameTimer::TimerEvent;

class StaticTimer {
public:
    static void Sync(int32_t syncFrame = 0);
    static UINT32 GetFps();
    static float GetDeltaTime();
    static float GetTimeFromStart();

    static void Update();

    static void PushTimerEvent(GameTimer::EventCallBack&& callback, GameTimer::Duration time, int32_t loopCount);

private:
    inline static GameTimer mTimer{ };
};
