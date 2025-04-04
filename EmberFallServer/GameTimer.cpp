#include "pch.h"
#include "GameTimer.h"

GameTimer::TimerEvent::TimerEvent(EventCallBack&& callback, TimePoint timeRegistered, Duration time, int32_t loopCount)
    : mFunction{ callback }, mTimeRegistered{ timeRegistered }, mDelay{ time }, mDelayOrigin{ time }, mLoopCount{ loopCount } { }

TimerEvent::~TimerEvent() { }

TimerEvent::TimerEvent(TimerEvent&& other) noexcept { 
    mFunction = std::move(other.mFunction);
    mTimeRegistered = GameTimer::Clock::now();
    mDelay = other.mDelay - std::chrono::duration_cast<Duration>(GameTimer::Clock::now() - other.mTimeRegistered);
    mDelayOrigin = other.mDelayOrigin;
    mLoopCount = other.mLoopCount;
}

TimerEvent& TimerEvent::operator=(TimerEvent&& other) noexcept {
    mFunction = std::move(other.mFunction);
    mTimeRegistered = GameTimer::Clock::now();
    mDelay = other.mDelay - std::chrono::duration_cast<Duration>(GameTimer::Clock::now() - other.mTimeRegistered);
    mDelayOrigin = other.mDelayOrigin;
    mLoopCount = other.mLoopCount;
    return *this;
}

bool TimerEvent::operator<(const TimerEvent& right) const {
    auto leftExcutionTime = mTimeRegistered + std::chrono::duration_cast<Clock::duration>(mDelay);
    auto rightExcutionTime = right.mTimeRegistered + std::chrono::duration_cast<Clock::duration>(right.mDelay);
    return  leftExcutionTime > rightExcutionTime;
}

GameTimer::EventCallBack GameTimer::TimerEvent::GetFunction() const {
    return mFunction;
}

GameTimer::Duration GameTimer::TimerEvent::GetDuration() const {
    return mDelayOrigin;
}

int32_t GameTimer::TimerEvent::GetLoopCount() const {
    return mLoopCount;
}

bool GameTimer::TimerEvent::IsTimeSinceResistered(std::chrono::time_point<Clock> time) const {
    return time > (mTimeRegistered + std::chrono::duration_cast<Clock::duration>(mDelay));
}

void GameTimer::TimerEvent::Excute() const {
    mFunction();
}

GameTimer::GameTimer()
    : mPrevPoint{ Clock::now() }, mPointSinceStart{ Clock::now() } { }

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
    ++mFpsCount;
    auto curPoint = Clock::now();

    mDeltaTime = std::chrono::duration_cast<Duration>(curPoint - mPrevPoint).count() / 1000.0f;
    mFpsCounter += mDeltaTime;

    mPrevPoint = curPoint;

    if (mDeltaTime < mSyncRatio) {
        auto sleepMilliSec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<float>(mSyncRatio - mDeltaTime));
        std::this_thread::sleep_for(sleepMilliSec);
        mDeltaTime = mSyncRatio;
    }

    ProcessEvent();
}

void GameTimer::ProcessEvent() {
    auto timeNow = Clock::now();
    while (not mEventQueue.empty()) {
        decltype(auto) topEvent = mEventQueue.top();
        if (false == topEvent.IsTimeSinceResistered(timeNow)) {
            break;
        }

        topEvent.Excute();

        auto nextLoopCount = topEvent.GetLoopCount() - 1;
        if (nextLoopCount > 0) {
            PushTimerEvent(topEvent.GetFunction(), topEvent.GetDuration(), nextLoopCount);
        }

        mEventQueue.pop();
    }
}

void GameTimer::PushTimerEvent(EventCallBack&& callback, Duration time, int32_t loopCount) {
    mEventQueue.emplace(std::move(callback), Clock::now(), time, loopCount);
}

void StaticTimer::Sync(int32_t syncFrame) {
    mTimer.Sync(syncFrame);
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
    mTimer.Update();
}

void StaticTimer::PushTimerEvent(GameTimer::EventCallBack&& callback, GameTimer::Duration time, int32_t loopCount) {
    mTimer.PushTimerEvent(std::move(callback), time, loopCount);
}

void StaticTimer::PushTimerEvent(GameTimer::EventCallBack&& callback, float time, int32_t loopCount) {
    auto duration = std::chrono::duration_cast<GameTimer::Duration>(std::chrono::duration<float>(time));
    mTimer.PushTimerEvent(std::move(callback), duration, loopCount);
}
