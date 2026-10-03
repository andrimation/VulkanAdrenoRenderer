# pragma once

#include "../VulkanCommon.h"

class WindowGLFW;

class Vk_Context
{
public:
	Vk_Context() {};

	void InitVkContext(WindowGLFW* InWindow)
	{
		Window = InWindow;

		CreateInstance();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
	}

private:

	void CreateInstance();
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();

	std::vector<const char*> GetRequiredInstanceExtensions();
	bool IsDeviceSuitable(vk::raii::PhysicalDevice const& InPhysicalDevice);  
	vk::raii::PhysicalDevice* ChoosePhysicalDeviceByScore(std::vector<vk::raii::PhysicalDevice>& physicalDevices);

	WindowGLFW* Window;

public:
	vk::raii::Context context;
	vk::raii::Instance instance = nullptr;
	vk::raii::SurfaceKHR surface = nullptr;
	vk::raii::PhysicalDevice physicalDevice = nullptr;
	vk::raii::Device logicalDevice = nullptr;
	vk::raii::Queue graphicsQueue = nullptr;
	uint32_t graphicsQueueIndex = ~0;
};