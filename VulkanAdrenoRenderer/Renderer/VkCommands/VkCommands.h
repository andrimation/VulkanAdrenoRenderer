#pragma once

#include "../VulkanCommon.h"

class Vk_Context;
class Vk_SwapChain;
class Vk_Pipeline;
class Vk_Buffers;
class Vk_ProfilerGPU;

class Vk_Commands
{
public:
	Vk_Commands() = default;

	void InitVkCommands(
		Vk_Context* InContext, 
		Vk_SwapChain* InSwapChain, 
		Vk_Buffers* InVertexBuffer,
		Vk_Pipeline* InPipeline,
		std::vector<vk::raii::DescriptorSet>* InDescriptorSets,
		Vk_ProfilerGPU* InProfiler
	);

	void RecordCommandBuffer(uint32_t imageIndex, uint32_t frameIndex);

private:
	void CreateCommandPool();
	void CreateCommandBuffers();
	void TransitionImageLayout(
		uint32_t imageIndex, 
		uint32_t frameIndex,
		vk::ImageLayout oldLayout, 
		vk::ImageLayout newLayout,
		vk::AccessFlags2 srcAccessMask, 
		vk::AccessFlags2 dstAccessMask,
		vk::PipelineStageFlags2 srcStageMask, 
		vk::PipelineStageFlags2 dstStageMask
	);

	Vk_Context* Context;
	Vk_SwapChain* SwapChain;
	Vk_Buffers* Buffers;
	Vk_Pipeline* Pipeline;
	std::vector<vk::raii::DescriptorSet>* DescriptorSets;

	Vk_ProfilerGPU* Profiler;

public:

	static constexpr uint32_t MaxFramesInFlight = 2;
	vk::raii::CommandPool commandPool = nullptr;
	std::vector<vk::raii::CommandBuffer> commandBuffers;

};