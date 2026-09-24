#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

class Vk_ProfilerCPU
{
public:

	explicit Vk_ProfilerCPU(uint32_t InAverageWindow = 120);
	void BeginFrame();

	// Kończy pomiar i zwraca czas ostatniej klatki w ms.
	double EndFrame();
	double GetLastFrameTimeMs() const
	{
		return LastFrameTimeMs;
	}

	double GetAverageFrameTimeMs() const;
	double GetAverageFPS() const;

private:

	using Clock = std::chrono::steady_clock;
	void PushSample(double InMilliseconds);

	Clock::time_point StartTime;
	bool bIsMeasuring = false;
	double LastFrameTimeMs = 0.0;
	std::vector<double> Samples;

	uint32_t NextSampleIndex = 0;
	uint32_t NumValidSamples = 0;

	double SamplesSum = 0.0;
};