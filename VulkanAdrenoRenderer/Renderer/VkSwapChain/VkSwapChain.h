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
		Context = InContext;
		Window = InWindow;
		Init();
	};

	void Init()
	{
		CreateSwapChain();
		CreateImageViews();
	}

	void RecreateVkSwapChain()
	{
		CleanupSwapChain();
		Init();
	};

private:
	void CreateSwapChain();
	void CreateImageViews();
	void CleanupSwapChain();
	vk::SurfaceFormatKHR ChooseSwapChainSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats);
	vk::PresentModeKHR ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);
	vk::Extent2D ChooseSwapChainExtent(vk::SurfaceCapabilitiesKHR const& capabilities);
	uint32_t ChooseSwapChainMinImageCount(vk::SurfaceCapabilitiesKHR const& capabilities);

	Vk_Context* Context;
	WindowGLFW* Window;


public:
	vk::raii::SwapchainKHR swapChain = nullptr;
	std::vector<vk::Image> swapChainImages;
	std::vector<vk::raii::ImageView> swapChainImageViews;  // <- image views służą do dostępu do vk::Images - zawietają informację jak dany Image powinien być używany
	vk::SurfaceFormatKHR swapChainSurfaceFormat;
	vk::Extent2D swapChainExtent;
};