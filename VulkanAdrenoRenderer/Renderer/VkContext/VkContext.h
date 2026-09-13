# pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS  

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

// Enablujemy Validation layers
const std::vector<char const*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif


class WindowGLFW;

class VkContext
{
public:
	VkContext() {};

	void InitVkContext(WindowGLFW* InWindow)
	{
		CreateInstance();
		CreateSurface(InWindow);
		PickPhysicalDevice();
		CreateLogicalDevice();
	}

private:

	void CreateInstance();
	void CreateSurface(WindowGLFW* InWindow);
	void PickPhysicalDevice();
	void CreateLogicalDevice();

	std::vector<const char*> GetRequiredInstanceExtensions();
	bool IsDeviceSuitable(vk::raii::PhysicalDevice const& InPhysicalDevice);  
	vk::raii::PhysicalDevice* ChoosePhysicalDeviceByScore(std::vector<vk::raii::PhysicalDevice>& physicalDevices);

public:
	vk::raii::Context context;
	vk::raii::Instance instance = nullptr;
	vk::raii::SurfaceKHR surface = nullptr;
	vk::raii::PhysicalDevice physicalDevice = nullptr;
	vk::raii::Device logicalDevice = nullptr;
	vk::raii::Queue graphicsQueue = nullptr;
	uint32_t graphicsQueueIndex = ~0;
};