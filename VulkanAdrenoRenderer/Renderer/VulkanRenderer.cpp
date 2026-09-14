#include "VulkanRenderer.h"
#include "VulkanRenderer.h"
#include "VulkanRenderer.h"
#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer()
{

}

void VulkanRenderer::MainLoop()
{
	//glfwGetWindowSize(Renderer.window, &previousWidth, &prewiousHeight);  <- to do resize
	while (!glfwWindowShouldClose(Window.window))
	{
		glfwPollEvents();
		DrawFrame(&VulkanContext, &VulkanSwapChain, &VulkanPipeline);
	}
	VulkanContext.logicalDevice.waitIdle();

	Cleanup();
}

bool VulkanRenderer::DrawFrame(Vk_Context* InContext, Vk_SwapChain* InSwapChain, Vk_Pipeline* InPipeline)
{
	if (InContext->logicalDevice.waitForFences(*VulkanSynchronization.drawFences[VulkanSynchronization.frameIndex], vk::True, UINT64_MAX) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to wait for fence");
	}

	auto [result, imageIndex] = InSwapChain->swapChain.acquireNextImage(UINT64_MAX, *VulkanSynchronization.getImageCompleteSemaphores[VulkanSynchronization.frameIndex], nullptr);
	if (result == vk::Result::eErrorOutOfDateKHR)
	{
		return false;
	}
	if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("Failed to acquire swap chain image");
	}

	InContext->logicalDevice.resetFences(*VulkanSynchronization.drawFences[VulkanSynchronization.frameIndex]);
	VulkanCommands.RecordCommandBuffer(imageIndex, VulkanSynchronization.frameIndex,InSwapChain, InPipeline);

	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	const vk::SubmitInfo submitInfo{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*VulkanSynchronization.getImageCompleteSemaphores[VulkanSynchronization.frameIndex],
		.pWaitDstStageMask = &waitDestinationStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &*VulkanCommands.commandBuffers[VulkanSynchronization.frameIndex],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &*VulkanSynchronization.renderFinishedSemaphores[imageIndex]
	};

	InContext->graphicsQueue.submit(submitInfo, *VulkanSynchronization.drawFences[VulkanSynchronization.frameIndex]);

	const vk::PresentInfoKHR presentInfo{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*VulkanSynchronization.renderFinishedSemaphores[imageIndex],
		.swapchainCount = 1,
		.pSwapchains = &*InSwapChain->swapChain,
		.pImageIndices = &imageIndex
	};

	result = InContext->graphicsQueue.presentKHR(presentInfo);
	VulkanSynchronization.frameIndex = (VulkanSynchronization.frameIndex + 1) % VulkanCommands.MaxFramesInFlight;

	return result != vk::Result::eSuboptimalKHR && result != vk::Result::eErrorOutOfDateKHR;
}

void VulkanRenderer::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto appPtr = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
	appPtr->frameBufferResized = true;
}
