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

inline constexpr uint8_t GAME_ROOM_EVENT = 0xF0;

enum class TimerEventType : uint8_t {
    REMOVE_NPC,
    UPDATE_NPC,
    REMOVE_TRIGGER,
    SCENE_TRANSITION_COUNTDOWN = GAME_ROOM_EVENT,
    CHECK_GAME_CONDITION,
    CHECK_SESSION_HEART_BEAT
};

struct TimerEvent {
    uint16_t roomIdx{ };
    NetworkObjectIdType id{ };
    SysClock::time_point executeTime{ };
    TimerEventType eventType{ };

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