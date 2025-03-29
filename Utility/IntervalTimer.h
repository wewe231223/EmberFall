#pragma once 
#include <chrono>
#include <iostream>
#include <thread>

class IntervalTimer {
public:
    using Clock = std::chrono::high_resolution_clock;
public:
    IntervalTimer() = default;
	~IntervalTimer() = default;

public:
    inline void Start() {
        startTime = Clock::now();
    }

    inline void End() {
        endTime = Clock::now();
    }

    template <typename DurationType>
    inline double Elapsed() const {
        return std::chrono::duration_cast<DurationType>(endTime - startTime).count();
    }

    inline double Seconds() const {
        return Elapsed<std::chrono::duration<double>>();
    }

    inline double Milliseconds() const {
        return Elapsed<std::chrono::duration<double, std::milli>>();
    }

    inline double Microseconds() const {
        return Elapsed<std::chrono::duration<double, std::micro>>();
    }

    inline double Nanoseconds() const {
        return Elapsed<std::chrono::duration<double, std::nano>>();
    }

private:
    std::chrono::time_point<Clock> startTime;
    std::chrono::time_point<Clock> endTime;
};
