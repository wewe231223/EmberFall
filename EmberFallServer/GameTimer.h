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
using EventCallBack = std::function<void()>;

template <typename MyClock=Clock>
using TimePoint = typename MyClock::time_point;

inline constexpr uint8_t GAME_ROOM_EVENT = 0xF0;

struct TimerEvent {
    NetworkObjectIdType id{ };
    IoType eventType{ };
    ExtraInfo extraInfo{ };
};

class SimpleTimer {
public:
    SimpleTimer() = default;
    ~SimpleTimer() = default;

public:
    void UpdatePoint();
    float GetDeltaTime() const;

    TimePoint<Clock> GetPointNow();

private:
    TimePoint<Clock> mPrevPoint{ };
    TimePoint<Clock> mCurrPoint{ };
};