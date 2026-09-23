#pragma once

#include "../VulkanCommon.h"

class Vk_Context;
class Vk_SwapChain;
class Vk_Pipeline;
class Vk_VertexBuffer;

class Vk_Commands
{
public:
	Vk_Commands() = default;

	void InitVkCommands(Vk_Context* InContext, Vk_SwapChain* InSwapChain, Vk_VertexBuffer* InVertexBuffer);
	void RecordCommandBuffer(uint32_t imageIndex, uint32_t frameIndex, Vk_SwapChain* InSwapChain, Vk_Pipeline* InPipeline,Vk_VertexBuffer* InVertexBuffer);

private:
	void CreateCommandPool(Vk_Context* InContext);
	void CreateCommandBuffers(Vk_Context* InContext);
	void TransitionImageLayout(uint32_t imageIndex, uint32_t frameIndex,vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
		vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask,
		vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask,
		Vk_SwapChain* InSwapChain);

public:

	static constexpr uint32_t MaxFramesInFlight = 2;
	vk::raii::CommandPool commandPool = nullptr;
	std::vector<vk::raii::CommandBuffer> commandBuffers;
};