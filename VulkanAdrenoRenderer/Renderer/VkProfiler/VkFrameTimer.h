#pragma once

#include <chrono>

class Vk_FrameTimer
{
public:
	Vk_FrameTimer();

	// Wywołuj raz na początku każdej klatki.
	// Zwraca czas od poprzedniej klatki w sekundach.
	float Tick();

	// Zwraca true co InIntervalSeconds.
	bool HasIntervalElapsed(float InIntervalSeconds);

private:
	using Clock = std::chrono::steady_clock;

	Clock::time_point PreviousTime;

	float DeltaTime = 0.0f;
	float IntervalAccumulator = 0.0f;
};