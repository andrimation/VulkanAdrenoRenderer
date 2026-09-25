#include "VkProfilerCPU.h"

#include <cassert>

Vk_ProfilerCPU::Vk_ProfilerCPU(uint32_t InAverageWindow)
{
	assert(InAverageWindow > 0);
	Samples.resize(InAverageWindow, 0.0);
}

void Vk_ProfilerCPU::BeginFrame()
{
	assert(!bIsMeasuring);
	bIsMeasuring = true;
	StartTime = Clock::now();
}

double Vk_ProfilerCPU::EndFrame()
{
	assert(bIsMeasuring);
	const Clock::time_point EndTime = Clock::now();

	LastFrameTimeMs =
		std::chrono::duration<double, std::milli>(
			EndTime - StartTime
		).count();

	bIsMeasuring = false;
	PushSample(LastFrameTimeMs);

	return LastFrameTimeMs;
}

void Vk_ProfilerCPU::PushSample(double InMilliseconds)
{
	if (NumValidSamples < Samples.size())
	{
		++NumValidSamples;
	}
	else
	{
		// Wyrzucamy stary sample z sumy.
		SamplesSum -= Samples[NextSampleIndex];
	}

	Samples[NextSampleIndex] = InMilliseconds;
	SamplesSum += InMilliseconds;
	NextSampleIndex = (NextSampleIndex + 1) % static_cast<uint32_t>(Samples.size());
}

double Vk_ProfilerCPU::GetAverageFrameTimeMs() const
{
	if (NumValidSamples == 0)
	{
		return 0.0;
	}

	return SamplesSum / static_cast<double>(NumValidSamples);
}

double Vk_ProfilerCPU::GetAverageFPS() const
{
	const double AverageFrameTimeMs = GetAverageFrameTimeMs();
	if (AverageFrameTimeMs <= 0.0)
	{
		return 0.0;
	}

	return 1000.0 / AverageFrameTimeMs;
}