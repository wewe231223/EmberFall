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

void GameTimer::Sync(INT32 syncFrame) {
    if (0 == syncFrame) {
        mSyncRatio = 0.0f;
        return;
    }

    mSyncRatio = 1.0f / static_cast<float>(syncFrame);
}

void GameTimer::Update() {
    static UINT32 fpsCount{ };
    ++fpsCount;

    auto curPoint = Clock::now();

    mDeltaTime = std::chrono::duration_cast<Duration>(curPoint - mPrevPoint).count() / 1000.0f;
    mFpsCounter += mDeltaTime;
    if (1.0f <= mFpsCounter) {
        mFps = fpsCount;
        fpsCount = 0;
        mFpsCounter = 0.0f;
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "FPS: {}", mFps);
    }

    mPrevPoint = curPoint;

    if (mDeltaTime < mSyncRatio) {
        auto sleepMilliSec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<float>(mSyncRatio - mDeltaTime));
        std::this_thread::sleep_for(sleepMilliSec);
        mDeltaTime = mSyncRatio;
    }
}
