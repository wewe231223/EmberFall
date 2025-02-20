#pragma once 
#include <bitset>
#include <random>
#include "../Utility/Crash.h"
#include "../Utility/RandomEngine.h"
constexpr int TOTAL_SAMPLES = 1024;

// 비복원 샘플링을 위한 클래스 
class NonReplacementSampler {
public:
    NonReplacementSampler() = default;
    ~NonReplacementSampler() = default;

	inline static NonReplacementSampler& GetInstance() {
		static NonReplacementSampler instance;
		return instance;
	}

    // 비복원 샘플링 함수
    int Sample() {
		CrashExp(mCount < TOTAL_SAMPLES, "No more samples to give");
        int number;
        do {
            number = mDistribution(MersenneTwister);  // 난수 생성
        } while (mUsed.test(number));  // 이미 선택된 숫자인지 확인

        mUsed.set(number);  // 숫자를 선택된 것으로 마크
        ++mCount;
        return number;
    }
    
    void Free(int sign) {
        mUsed.reset(sign);
        --mCount;
    }

private:
    std::uniform_int_distribution<int> mDistribution{ 0,TOTAL_SAMPLES - 1 };
    std::bitset<TOTAL_SAMPLES> mUsed{};
    int mCount{ 0 };
};

