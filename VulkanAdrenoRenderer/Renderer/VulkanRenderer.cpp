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

		//drawFrame();
	}
	VulkanContext.logicalDevice.waitIdle();

	Cleanup();
}

void VulkanRenderer::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto appPtr = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
	appPtr->frameBufferResized = true;
}
