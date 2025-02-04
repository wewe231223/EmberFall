#include "pch.h"
#include "GameTimer.h"

TimerEvent::TimerEvent() { }

TimerEvent::~TimerEvent() { }

TimerEvent::TimerEvent(const TimerEvent& other) { }

TimerEvent::TimerEvent(TimerEvent&& other) noexcept { }

TimerEvent& TimerEvent::operator=(const TimerEvent& other) { 
    return *this;
}

TimerEvent& TimerEvent::operator=(TimerEvent&& other) noexcept {
    return *this;
}

GameTimer::GameTimer()
    : mPrevPoint{ Clock::now() } { }

GameTimer::~GameTimer() { }

UINT32 GameTimer::GetFps() const {
    return mFps;
}

float GameTimer::GetDeltaTime() const {
    return mDeltaTime;
}

void GameTimer::Update() {
    static UINT32 fpsCount{ };
    ++fpsCount;

    auto curPoint = Clock::now();

    mDeltaTime = std::chrono::duration_cast<Duration>(curPoint - mPrevPoint).count() / 1000.0f;
    mFpsCounter += mDeltaTime;
    if (1.0f <= mFpsCounter) {
        mFps = fpsCount;
        mFpsCounter = 0.0f;
    }

    mPrevPoint = curPoint;
}
