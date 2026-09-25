#include "VkFrameTimer.h"

Vk_FrameTimer::Vk_FrameTimer()
	: PreviousTime(Clock::now())
{
}

float Vk_FrameTimer::Tick()
{
	const auto CurrentTime = Clock::now();
	DeltaTime = std::chrono::duration<float>(CurrentTime - PreviousTime).count();

	PreviousTime = CurrentTime;
	IntervalAccumulator += DeltaTime;

	return DeltaTime;
}

bool Vk_FrameTimer::HasIntervalElapsed(float InIntervalSeconds)
{
	if (IntervalAccumulator < InIntervalSeconds)
	{
		return false;
	}

	IntervalAccumulator -= InIntervalSeconds;
	return true;
}