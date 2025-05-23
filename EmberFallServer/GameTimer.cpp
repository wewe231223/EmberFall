#include "pch.h"
#include "GameTimer.h"

void SimpleTimer::UpdatePoint() {
    mPrevPoint = mCurrPoint;
    mCurrPoint = Clock::now();
}

float SimpleTimer::GetDeltaTime() const {
    return std::chrono::duration_cast<std::chrono::microseconds>(mCurrPoint - mPrevPoint).count() / 1000'000.0f;
}

Clock::time_point SimpleTimer::GetPointNow() {
    return Clock::now();
}
