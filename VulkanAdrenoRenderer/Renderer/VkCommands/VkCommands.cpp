#include "VkCommands.h"

#include "../VkContext/VkContext.h"
#include "../VkPipeline/VkPipeline.h"
#include "../VkSwapChain/VkSwapChain.h"

void Vk_Commands::InitVkCommands(Vk_Context* InContext, Vk_SwapChain* InSwapChain)
{
	CreateCommandPool(InContext);
	CreateCommandBuffers(InContext);
}

void Vk_Commands::CreateCommandPool(Vk_Context* InContext)
{
	vk::CommandPoolCreateInfo commandPoolCreateInfo
	{
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = InContext->graphicsQueueIndex
	};

	commandPool = vk::raii::CommandPool(InContext->logicalDevice, commandPoolCreateInfo);
}

void Vk_Commands::CreateCommandBuffers(Vk_Context* InContext)
{
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo{
		.commandPool = commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = MaxFramesInFlight
	};

	commandBuffers = vk::raii::CommandBuffers(InContext->logicalDevice, commandBufferAllocateInfo);
}

void Vk_Commands::RecordCommandBuffer(uint32_t imageIndex,uint32_t frameIndex, Vk_SwapChain* InSwapChain, Vk_Pipeline* InPipeline)
{
	auto& commandBuffer = commandBuffers[frameIndex];
	commandBuffer.begin({});

	TransitionImageLayout(imageIndex, frameIndex,vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
		{}, vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		InSwapChain);

	vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	vk::RenderingAttachmentInfo attachmentInfo = {
		.imageView = InSwapChain->swapChainImageViews[imageIndex],
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor
	};

	vk::RenderingInfo renderingInfo = {
		.renderArea = {.offset = {0, 0}, .extent = InSwapChain->swapChainExtent},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachmentInfo
	};

	commandBuffer.beginRendering(renderingInfo);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *InPipeline->GetPipeline());
	commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(InSwapChain->swapChainExtent.width), static_cast<float>(InSwapChain->swapChainExtent.height), 0.0f, 1.0f));
	commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), InSwapChain->swapChainExtent));
	commandBuffer.draw(3, 1, 0, 0);
	commandBuffer.endRendering();

	TransitionImageLayout(imageIndex, frameIndex,vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite, {},
		vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe,
		InSwapChain);

	commandBuffer.end();
}

void Vk_Commands::TransitionImageLayout(uint32_t imageIndex, uint32_t frameIndex,vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
	vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask,
	vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask,
	Vk_SwapChain* InSwapChain)
{
	vk::ImageMemoryBarrier2 barrier = {
		.srcStageMask = srcStageMask,
		.srcAccessMask = srcAccessMask,
		.dstStageMask = dstStageMask,
		.dstAccessMask = dstAccessMask,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = InSwapChain->swapChainImages[imageIndex],
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	vk::DependencyInfo dependencyInfo = {
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier
	};

	commandBuffers[frameIndex].pipelineBarrier2(dependencyInfo);
}
