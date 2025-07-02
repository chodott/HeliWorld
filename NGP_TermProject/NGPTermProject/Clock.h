#pragma once
#include <chrono>
class Clock {
public:
	using time = std::chrono::high_resolution_clock;
	using second = std::chrono::duration<float>;

	Clock() : timePassed(0.f), timeStamp(time::now()) {}
	void Record() { timeStamp = time::now(); }
	float GetTimePassedFromLastUpdate() { return second(time::now() - timeStamp).count(); }
private:
	float timePassed;
	time::time_point timeStamp;
}