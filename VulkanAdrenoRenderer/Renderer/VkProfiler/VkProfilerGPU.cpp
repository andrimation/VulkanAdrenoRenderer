#include "VkProfilerGPU.h"

#include <array>
#include <cassert>
#include <stdexcept>

void Vk_ProfilerGPU::InitProfilerGPU(vk::PhysicalDevice InPhysicalDevice,vk::Device InDevice,uint32_t InFramesInFlight,uint32_t InAverageWindow)	
{
	Device = InDevice;
	FramesInFlight = InFramesInFlight;

	// dodać timestamp peroids !
	const vk::PhysicalDeviceProperties GPUProperties = InPhysicalDevice.getProperties();
	TimestampPeriodNs = GPUProperties.limits.timestampPeriod;

	assert(Device);
	assert(TimestampPeriodNs > 0.0f);
	assert(FramesInFlight > 0);
	assert(InAverageWindow > 0);

	vk::QueryPoolCreateInfo QueryPoolInfo{};

	QueryPoolInfo.queryType = vk::QueryType::eTimestamp;
	QueryPoolInfo.queryCount = FramesInFlight * 2;

	TimestampQueryPool = Device.createQueryPool(QueryPoolInfo);

	QueryWasWritten.resize(
		FramesInFlight,
		false
	);

	Samples.resize(
		InAverageWindow,
		0.0
	);
}

Vk_ProfilerGPU::~Vk_ProfilerGPU()
{
	if (Device && TimestampQueryPool)
	{
		Device.destroyQueryPool(TimestampQueryPool);
	}
}

void Vk_ProfilerGPU::BeginFrame(
	vk::CommandBuffer InCommandBuffer,
	uint32_t InFrameIndex
)
{
	assert(InFrameIndex < FramesInFlight);

	const uint32_t FirstQuery =
		InFrameIndex * 2;

	/*
	 * Query pool dla tego frame indexu jest już bezpieczny do użycia,
	 * ponieważ przed nagrywaniem tej klatki czekamy na jej fence.
	 */
	InCommandBuffer.resetQueryPool(
		TimestampQueryPool,
		FirstQuery,
		2
	);

	/*
	 * START GPU.
	 *
	 * Timestamp zostanie zapisany przez GPU podczas wykonywania
	 * command buffera.
	 */
	InCommandBuffer.writeTimestamp2(
		vk::PipelineStageFlagBits2::eTopOfPipe,
		TimestampQueryPool,
		FirstQuery
	);
}

void Vk_ProfilerGPU::EndFrame(
	vk::CommandBuffer InCommandBuffer,
	uint32_t InFrameIndex
)
{
	assert(InFrameIndex < FramesInFlight);

	const uint32_t EndQuery = InFrameIndex * 2 + 1;

	/*
	 * END GPU.
	 */
	InCommandBuffer.writeTimestamp2(
		vk::PipelineStageFlagBits2::eBottomOfPipe,
		TimestampQueryPool,
		EndQuery
	);

	QueryWasWritten[InFrameIndex] = true;
}

std::optional<double> Vk_ProfilerGPU::ReadFrame(
	uint32_t InFrameIndex
)
{
	assert(InFrameIndex < FramesInFlight);

	if (!QueryWasWritten[InFrameIndex])
	{
		/*
		 * Pierwsze użycie tego FrameIndex.
		 *
		 * Fence może być początkowo signaled, ale żadnego timestampu
		 * jeszcze nigdy nie zapisaliśmy.
		 */
		return std::nullopt;
	}

	const uint32_t FirstQuery =
		InFrameIndex * 2;

	std::array<uint64_t, 2> Timestamps{};

	const vk::Result Result =
		Device.getQueryPoolResults(
			TimestampQueryPool,
			FirstQuery,
			2,
			sizeof(Timestamps),
			Timestamps.data(),
			sizeof(uint64_t),
			vk::QueryResultFlagBits::e64
		);

	if (Result == vk::Result::eNotReady)
	{
		return std::nullopt;
	}

	if (Result != vk::Result::eSuccess)
	{
		throw std::runtime_error(
			"Failed to read Vulkan GPU timestamp queries"
		);
	}

	const uint64_t StartTimestamp = Timestamps[0];
	const uint64_t EndTimestamp = Timestamps[1];
	const uint64_t DeltaTicks = EndTimestamp - StartTimestamp;

	/*
	 * timestampPeriod jest wyrażony w nanosekundach na jeden tick.
	 */
	const double Nanoseconds = static_cast<double>(DeltaTicks) * static_cast<double>(TimestampPeriodNs);

	LastFrameTimeMs =
		Nanoseconds / 1'000'000.0;

	PushSample(LastFrameTimeMs);

	QueryWasWritten[InFrameIndex] = false;

	return LastFrameTimeMs;
}

void Vk_ProfilerGPU::PushSample(double InMilliseconds)
{
	if (NumValidSamples < Samples.size())
	{
		++NumValidSamples;
	}
	else
	{
		SamplesSum -= Samples[NextSampleIndex];
	}

	Samples[NextSampleIndex] =
		InMilliseconds;

	SamplesSum +=
		InMilliseconds;

	NextSampleIndex =
		(NextSampleIndex + 1) %
		static_cast<uint32_t>(Samples.size());
}

double Vk_ProfilerGPU::GetAverageFrameTimeMs() const
{
	if (NumValidSamples == 0)
	{
		return 0.0;
	}

	return SamplesSum /
		static_cast<double>(NumValidSamples);
}