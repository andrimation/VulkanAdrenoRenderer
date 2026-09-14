# pragma once

#include "../VulkanCommon.h"

class Vk_Context;
class WindowGLFW;

class Vk_SwapChain
{
public:

	Vk_SwapChain() {};

	void InitVkSwapChain(Vk_Context* InContext,WindowGLFW* InWindow)
	{
		CreateSwapChain(InContext,InWindow);
		CreateImageViews(InContext);
	};

	void RecreateVkSwapChain(Vk_Context* InContext, WindowGLFW* InWindow)
	{
		CleanupSwapChain();
		InitVkSwapChain(InContext, InWindow);
	};

private:
	void CreateSwapChain(Vk_Context* InContext, WindowGLFW* InWindow);
	void CreateImageViews(Vk_Context* InContext);
	void CleanupSwapChain();
	vk::SurfaceFormatKHR ChooseSwapChainSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats);
	vk::PresentModeKHR ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);
	vk::Extent2D ChooseSwapChainExtent(vk::SurfaceCapabilitiesKHR const& capabilities, WindowGLFW* InWindow);
	uint32_t ChooseSwapChainMinImageCount(vk::SurfaceCapabilitiesKHR const& capabilities);


public:
	vk::raii::SwapchainKHR swapChain = nullptr;
	std::vector<vk::Image> swapChainImages;
	std::vector<vk::raii::ImageView> swapChainImageViews;  // <- image views służą do dostępu do vk::Images - zawietają informację jak dany Image powinien być używany
	vk::SurfaceFormatKHR swapChainSurfaceFormat;
	vk::Extent2D swapChainExtent;
};