#pragma once 
#include <chrono>
#include <functional>
#include <set>
#include <array>
#include <numeric>

/*
*  Time 으로 구현할 것들
*
1. Time.GetDeltaTime (V) ( GTime:: scaled / unscaled )
설명: 마지막 프레임과 현재 프레임 사이의 시간을 초 단위로 반환합니다.
용도: 프레임 속도에 무관한 이동, 애니메이션, 물리 시뮬레이션 등에 사용됩니다.
2. Time.GetTimeSinceStarted (V)  ( GTime:: scaled / unscaled )
설명: 게임이 시작된 후 경과한 시간을 초 단위로 반환합니다. Time.timeScale의 영향을 받습니다.
용도: 게임 시작 후 경과 시간 기반의 이벤트 트리거에 사용됩니다.
3. Time.SetTimeScale / GetTimeScale  (V)
설명: 게임의 전반적인 시간 흐름 속도를 제어합니다.
용도: 게임의 속도 조절, 일시 정지, 슬로우 모션 효과 등을 구현할 때 사용됩니다.
4. Time.GetSmoothDeltaTime ( V )
설명: 최근 몇 프레임 동안의 deltaTime의 평균을 반환합니다.
용도: 애니메이션이나 이동의 부드러움을 유지하기 위해 사용됩니다.
5. Time.GetFrameCount ( V )
설명: 게임이 시작된 이후부터 렌더링된 프레임 수를 반환합니다.
용도: 특정 프레임에서 이벤트를 발생시키거나, 프레임 기반 로직을 구현할 때 사용됩니다.
6. Time.GetTimeSinceSceneStarted ( V )
설명: 현재 씬(Scene)이 로드된 이후 경과한 시간을 반환합니다.
용도: 특정 씬 내에서의 경과 시간 기반 로직을 처리할 때 사용됩니다.
7. Time.AddEvent ( V )
설명: 특정 시간이 지난 후에 이벤트를 발생시키도록 스케줄링합니다.
	일회성으로 이벤트를 발생시키고 싶다면, 이벤트 Callable 에 false 를 리턴하도록 하면 되고, 주기적으로 이벤트를 발생시키고 싶아면, 이벤트 Callable 에 true 를 리턴하도록 하면 됩니다.
용도: 특정 시간에 이벤트를 발생시키거나, 반복적인 이벤트를 처리할 때 사용됩니다.
*/
template<typename T>
concept TimeUnit = std::chrono::_Is_duration_v<T>;

using namespace std::chrono_literals;

class GTime {
public:
	using clock = std::chrono::high_resolution_clock;
	using rep = double;
	using period = std::nano;
	using time_point = clock::time_point;
	using duration = std::chrono::duration<double, period>;
private:
	//sceduled event 가 우선되는 문제가 있다. 
	struct Event {
		Event(std::chrono::time_point<clock> time, std::chrono::nanoseconds timeout, std::function<bool()>&& callBack) {
			mTimeout = timeout;
			mInvokeTime = time;
			mCallBack = callBack;
		}

		Event(const Event& rhs) = default;
		Event(Event&& rhs) noexcept = default;
		Event& operator=(const Event& rhs) = default;
		Event& operator=(Event&& rhs) noexcept = default;

		bool operator<(const Event& rhs) const {
			return mInvokeTime < rhs.mInvokeTime;
		}

		std::chrono::time_point<clock> mInvokeTime{};
		std::chrono::nanoseconds mTimeout{};
		std::function<bool()> mCallBack{ []() {return false; } };
	};

public:
	enum class scaled {
		result_time_scaled,
		result_time_unscaled,
	};
public:
	GTime();
	~GTime();

	template<typename ResultTy = double, TimeUnit Tu = std::chrono::seconds>
	[[nodiscard]]
	ResultTy GetDeltaTime(scaled sc = scaled::result_time_unscaled) {
		if (sc == scaled::result_time_scaled) {
			return std::chrono::duration_cast<std::chrono::duration<ResultTy, typename Tu::period>>(mDeltaTime * mTimeScale).count();
		}
		// scaled::result_time_unscaled
		return std::chrono::duration_cast<std::chrono::duration<ResultTy, typename Tu::period>>(mDeltaTime).count();
	}

	template<typename ResultTy = double, TimeUnit Tu = std::chrono::seconds>
	[[nodiscard]]
	ResultTy GetTimeSinceStarted(scaled sc = scaled::result_time_unscaled) {
		if (sc == scaled::result_time_scaled) {
			return std::chrono::duration_cast<std::chrono::duration<ResultTy, typename Tu::period>>(mScaledStarted).count();
		}
		// scaled::result_time_unscaled
		duration AbsoluteElapsed = clock::now() - mAbsoluteStarted;
		return std::chrono::duration_cast<std::chrono::duration<ResultTy, typename Tu::period>>(AbsoluteElapsed).count();
	}

	template<typename ResultTy = double, TimeUnit Tu = std::chrono::seconds>
	[[nodiscard]]
	ResultTy GetTimeSinceSceneStarted() {
		duration Elapsed = clock::now() - mSceneStarted;
		return std::chrono::duration_cast<std::chrono::duration<ResultTy, typename Tu::period>>(Elapsed).count();
	}

	template<typename ResultTy = double, TimeUnit Tu = std::chrono::seconds>
	[[nodiscard]]
	ResultTy GetSmoothDeltaTime() {
		auto sumofSamples = std::accumulate(mDeltaTimeBuffer.begin(), mDeltaTimeBuffer.end(), duration::zero(),
			[](const duration& a, const duration& b) {
				if (b.count() <= 0.0)
					return a;
				return a + b;
			});
		return std::chrono::duration_cast<std::chrono::duration<ResultTy, typename Tu::period>>(sumofSamples / mDeltaTimeBufferSize).count();
	}
	/// <summary>
	/// 이 함수에는 bool 형을 반환하는 Callable 을 넣어주시기 바랍니다. 
	/// 해당 Callable 이 true 를 반환하면 그 다음 time_point 에 다시 Event 가 호출됩니다. 
	/// 해당 Callable 이 false 를 반환하면 그 다음 time_point 에 Event 가 삭제됩니다.
	/// </summary>
	/// <typeparam name="rep"></typeparam>
	/// <typeparam name="period"></typeparam>
	/// <param name="time">-그 다음 이벤트가 발동하기 까지 필요한 시간입니다.</param>
	/// <param name="callBack">-이벤트 발동 시 호출할 함수입니다.</param>
	template<typename rep, typename period>
	void AddEvent(std::chrono::duration<rep, period> time, std::function<bool()>&& callBack) {
		mEvents.emplace(clock::now() + time, std::chrono::duration_cast<std::chrono::nanoseconds>(time), std::move(callBack));
	}

	time_point Now() const;

	[[maybe_unused]]
	double SetTimeScale(double scale = 1.0);
	double GetTimeScale();
	uint64_t GetFrameCount();
	// 각 프레임이 시작될 때마다 호출함 
	void AdvanceTime();
	// Scene 이 시작될 때 호출 
	void StartSceneTime();
private:
	void UpdateDeltaTime();
	void AddScaledStarted();
	void SampleDeltaTime();
	bool PopEvent();
	void CheckEvent();
private:
	duration				mDeltaTime{};
	duration				mScaledStarted{};

	uint64_t				mFrameCount{ 0 };

	static constexpr unsigned int mDeltaTimeBufferSize = 10;
	unsigned int mDeltaTimeSampleingIndex = 0;
	std::array<duration, mDeltaTimeBufferSize> mDeltaTimeBuffer{};


	time_point				mPrev{ clock::now() };
	time_point 				mSceneStarted{ clock::now() };
	const time_point		mAbsoluteStarted{ clock::now() };
	double					mTimeScale{ 1.0 };

	std::set<Event>			mEvents{};
};


extern GTime Time;