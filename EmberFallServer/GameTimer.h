#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameTimer.h
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

using Clock = std::chrono::high_resolution_clock;
using SysClock = std::chrono::system_clock;
using TimePeriod = std::milli;
using Duration = std::chrono::duration<float, TimePeriod>;
using TimePoint = Clock::time_point;
using EventCallBack = std::function<void()>;

struct TimerEvent {
    NetworkObjectIdType id;
    SysClock::time_point executeTime;

    constexpr bool operator<(const TimerEvent& other) const {
        return executeTime > other.executeTime;
    }
};

class SimpleTimer {
public:
    SimpleTimer() = default;
    ~SimpleTimer() = default;

public:
    void UpdatePoint();
    float GetDeltaTime() const;

    Clock::time_point GetPointNow();

private:
    Clock::time_point mPrevPoint{ };
    Clock::time_point mCurrPoint{ };
};