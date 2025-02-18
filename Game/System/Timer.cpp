#include "pch.h"
#include "Timer.h"

GTime::GTime() {
}

GTime::~GTime() {
}

GTime::time_point GTime::Now() const {
	return clock::now();
}

double GTime::SetTimeScale(double scale) {
	auto temp = mTimeScale;
	mTimeScale = scale;
	return temp;
}


double GTime::GetTimeScale() {
	return mTimeScale;
}

uint64_t GTime::GetFrameCount() {
	return mFrameCount;
}


// 각 프레임이 시작될 때마다 호출함 
void GTime::AdvanceTime() {
	GTime::UpdateDeltaTime();
	GTime::SampleDeltaTime();
	GTime::AddScaledStarted();
	GTime::CheckEvent();
}

// Scene 이 시작될 때 호출 
void GTime::StartSceneTime() {
	mSceneStarted = clock::now();
}

void GTime::UpdateDeltaTime() {
	auto now = clock::now();
	mDeltaTime = std::chrono::duration_cast<duration>(now - mPrev);
	mPrev = now;
	mFrameCount++;
}
void GTime::AddScaledStarted() {
	mScaledStarted += mDeltaTime * mTimeScale;
}

void GTime::SampleDeltaTime() {
	mDeltaTimeBuffer[mDeltaTimeSampleingIndex] = mDeltaTime * mTimeScale;
	mDeltaTimeSampleingIndex = (mDeltaTimeSampleingIndex + 1) % mDeltaTimeBufferSize;
}

bool GTime::PopEvent() {
	Event ev = *mEvents.begin();

	if (ev.mInvokeTime < clock::now()) {
		if (std::invoke(ev.mCallBack)) {
			mEvents.emplace(clock::now() + ev.mTimeout, std::move(ev.mTimeout), std::move(ev.mCallBack));
		}
		mEvents.erase(mEvents.begin());

		return true;
	}
	return false;
}

void GTime::CheckEvent() {
	if (mEvents.empty()) {
		return;
	}
	while (GTime::PopEvent());
}

GTime Time{};