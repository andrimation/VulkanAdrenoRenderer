#pragma once

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <optional>
#include <vector>

class Vk_ProfilerGPU
{
public:

	Vk_ProfilerGPU() {};

	void InitProfilerGPU(
		vk::PhysicalDevice InPhysicalDevice,
		vk::Device InDevice,
		uint32_t InFramesInFlight,
		uint32_t InAverageWindow = 120
	);

	~Vk_ProfilerGPU();

	Vk_ProfilerGPU(const Vk_ProfilerGPU&) = delete;
	Vk_ProfilerGPU& operator=(const Vk_ProfilerGPU&) = delete;

	// Nagrywa START timestamp do command buffera.
	void BeginFrame(
		vk::CommandBuffer InCommandBuffer,
		uint32_t InFrameIndex
	);

	// Nagrywa END timestamp do command buffera.
	void EndFrame(
		vk::CommandBuffer InCommandBuffer,
		uint32_t InFrameIndex
	);

	// Wywołuj PO waitForFences() dla tego FrameIndex.
	//
	// nullopt oznacza, że dla tego frame indexu nie ma jeszcze
	// poprzedniego pomiaru do odczytania.
	std::optional<double> ReadFrame(uint32_t InFrameIndex);

	double GetLastFrameTimeMs() const
	{
		return LastFrameTimeMs;
	}

	double GetAverageFrameTimeMs() const;

private:

	void PushSample(double InMilliseconds);

	vk::Device Device;

	vk::QueryPool TimestampQueryPool;

	float TimestampPeriodNs = 0.0f;

	uint32_t FramesInFlight = 0;

	std::vector<bool> QueryWasWritten;

	double LastFrameTimeMs = 0.0;

	std::vector<double> Samples;

	uint32_t NextSampleIndex = 0;
	uint32_t NumValidSamples = 0;

	double SamplesSum = 0.0;
};