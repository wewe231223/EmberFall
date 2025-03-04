#include "pch.h"
#include "GameTimer.h"

GameTimer::TimerEvent::TimerEvent(EventCallBack&& callback, std::chrono::time_point<Clock> time) 
    : mFunction{ callback }, mTimeRegistered{ time } { }

TimerEvent::~TimerEvent() { }

TimerEvent::TimerEvent(const TimerEvent& other) { }

TimerEvent::TimerEvent(TimerEvent&& other) noexcept { }

TimerEvent& TimerEvent::operator=(const TimerEvent& other) { 
    return *this;
}

TimerEvent& TimerEvent::operator=(TimerEvent&& other) noexcept {
    return *this;
}

bool TimerEvent::operator<(const TimerEvent& right) const {
    return mTimeRegistered < right.mTimeRegistered;
}

GameTimer::GameTimer()
    : mPrevPoint{ Clock::now() }, mPointSinceStart{ Clock::now()} { }

GameTimer::~GameTimer() { }

UINT32 GameTimer::GetFps() const {
    return mFps;
}

float GameTimer::GetDeltaTime() const {
    return mDeltaTime;
}

float GameTimer::GetTimeFromStart() const{
    return std::chrono::duration_cast<Duration>(Clock::now() - mPointSinceStart).count() / 1000.0f;
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

void GameTimer::PushTimerEvent(EventCallBack&& callback, Duration time) {
    mEventQueue.emplace(std::move(callback), Clock::now() + std::chrono::duration_cast<Clock::duration>(time));
}

UINT32 StaticTimer::GetFps() {
    return mTimer.GetFps();
}

float StaticTimer::GetDeltaTime() {
    return mTimer.GetDeltaTime();
}

float StaticTimer::GetTimeFromStart() {
    return mTimer.GetTimeFromStart();
}

void StaticTimer::Update() {
    mTimer.Sync(0);
    mTimer.Update();
}
