#include "VkContext.h"
#include <iostream>
#include <map>


#define GLFW_INCLUDE_VULKAN // czyli w glfw3.h jest #if dedined(GLFW_INCLUDE_VULCAN) -> 
#include <GLFW/glfw3.h> 

#include "GLFWWindow/WindowGLFW.h"

void VkContext::CreateInstance()
{
	constexpr vk::ApplicationInfo appInfo
	{
		.pApplicationName = "Hello Triangle",
		.applicationVersion = VK_MAKE_VERSION(1,0,0),
		.pEngineName = "No engine",
		.engineVersion = VK_MAKE_VERSION(1,0,0),
		.apiVersion = vk::ApiVersion14
	};

	auto requiredGLFWExtensions = GetRequiredInstanceExtensions(); 
	auto extensionProperties = context.enumerateInstanceExtensionProperties();

	std::cout << "All available Vulcan Extensions" << "\n";
	for (const auto& extension : extensionProperties)
	{
		std::cout << "Extension:  " << extension.extensionName << "\n";
	}
	std::cout << "\n\nAll requested Vulcan Extensions" << "\n";
	for (uint32_t i = 0; i < requiredGLFWExtensions.size(); i++)
	{
		std::cout << "Extension:  " << requiredGLFWExtensions[i] << "\n";
	}
	std::cout << "\n\n";

	for (uint32_t i = 0; i < requiredGLFWExtensions.size(); i++)
	{   
		if (std::ranges::none_of(extensionProperties, [glfwExtension = requiredGLFWExtensions[i]](auto const& extensionProperty)
			{
				return strcmp(extensionProperty.extensionName, glfwExtension) == 0;
			}))
		{
			throw std::runtime_error("Required GLFW extension not supported: " + std::string(requiredGLFWExtensions[i]));
		}
	}

	std::vector<char const*> requiredLayers;
	if (enableValidationLayers)
	{
		requiredLayers.assign(validationLayers.begin(), validationLayers.end());
	}

	auto layerProperties = context.enumerateInstanceLayerProperties();  // Obsługiwane layersy
	auto unsupportedLayerIt = std::ranges::find_if(requiredLayers, [&layerProperties](auto const& requiredLayer)
		{
			return std::ranges::none_of(layerProperties, [requiredLayer](const auto& layerProperty)
				{
					return strcmp(layerProperty.layerName, requiredLayer) == 0;
				});
		});

	if (unsupportedLayerIt != requiredLayers.end())
	{
		throw std::runtime_error("Required layer unsuported:  " + std::string(*unsupportedLayerIt));
	}

	vk::InstanceCreateInfo instanceInfo
	{
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
		.ppEnabledLayerNames = requiredLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(requiredGLFWExtensions.size()),
		.ppEnabledExtensionNames = requiredGLFWExtensions.data()  // robimy .dat() bo Vulkan nie zna std::vector -> zna tablice w stylu C, więc 
	};

	instance = vk::raii::Instance(context, instanceInfo);

	// Kod poniżej pozwala sprawdzić czy wszystko poszło gładko.
	/*
	try {
		vk::raii::Context context;
		vk::raii::Instance instance(context, vk::InstanceCreateInfo{});
		vk::raii::PhysicalDevice physicalDevice = instance.enumeratePhysicalDevices().front();

		auto devices = instance.enumeratePhysicalDevices();
		vk::raii::Device device(physicalDevice, vk::DeviceCreateInfo{});
		vk::raii::Buffer buffer(device, vk::BufferCreateInfo{});
	}
	catch(const vk::SystemError& e)
	{
		std::cerr << "Vulkan Error:   " << e.what() << "\n";
		return;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error:   " << e.what() << "\n";
		return;
	}
	*/
}

void VkContext::CreateSurface(WindowGLFW* InWindow)
{
	VkSurfaceKHR _surface;  
	if (glfwCreateWindowSurface(*instance, InWindow->window, nullptr, &_surface) != 0)
	{
		throw std::runtime_error("Failed to create window surface");
	}
	surface = vk::raii::SurfaceKHR(instance, _surface); 
}

void VkContext::PickPhysicalDevice()
{
	auto physicalDevices = instance.enumeratePhysicalDevices();
	if (physicalDevices.empty())
	{
		throw std::runtime_error("failed to find GPU with Vulkan support!");
	}

	physicalDevice = *ChoosePhysicalDeviceByScore(physicalDevices);
	bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;
	std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

	bool supportsGraphics = std::ranges::any_of(queueFamilies, [](const auto& queueFamily)
		{
			return !!(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics);
		});

	bool supportsCompute = std::ranges::any_of(queueFamilies, [](const auto& queueFamily)
		{
			return !!(queueFamily.queueFlags & vk::QueueFlagBits::eCompute);
		});

	std::vector<const char*> requiredExtensions = { vk::KHRSwapchainExtensionName };
	auto availableDeviceExtension = physicalDevice.enumerateDeviceExtensionProperties();

	bool areAllExtensionsSupportedByGPU = std::ranges::all_of(requiredExtensions, [&availableDeviceExtension](const auto& requiredExtension)
		{
			return std::ranges::any_of(availableDeviceExtension, [&requiredExtension](const auto& availableExtension)
				{
					return strcmp(availableExtension.extensionName, requiredExtension) == 0;
				});
		});

	auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

	const vk::PhysicalDeviceFeatures coreFeatures = physicalDevice.getFeatures();
	bool idDepthClampAvailable = coreFeatures.depthClamp;

	bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceFeatures2>().features.depthClamp &&  // depth clamp sam dodałem żeby sprawdzić jak się włącze featury karty  <- tu nam odpowiada czy karta fizycznie obsługuje ( 
		features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
		features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

	auto Vulkan13Features = features.template get<vk::PhysicalDeviceVulkan13Features>();
	bool supportsVulkan13Features = Vulkan13Features.dynamicRendering && Vulkan13Features.synchronization2;

	if (supportsVulkan1_3 && supportsGraphics && supportsCompute && areAllExtensionsSupportedByGPU && supportsRequiredFeatures && supportsVulkan13Features)
	{
		std::cout << "Picked PhysicalDevice:   " << physicalDevice.getProperties().deviceName << "\n";
	}
	else
	{
		std::cout << "PhysicalDevice:   " << physicalDevice.getProperties().deviceName << "was picked as most efficient, but doesnt meet all criteria.No device picked" << "\n";
		physicalDevice = nullptr;
		std::runtime_error("Physical Device pick error");
	}
}

void VkContext::CreateLogicalDevice()
{
	// DeviceQueueCreateInfo 
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics && physicalDevice.getSurfaceSupportKHR(i, *surface))
		{
			graphicsQueueIndex = i;
			break;
		}
	}
	if (graphicsQueueIndex == ~0)
	{
		throw std::runtime_error("Graphics queue not found");
	}

	float queuePriority = 0.5f;  
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
													.queueFamilyIndex = graphicsQueueIndex,
													.queueCount = 1,
													.pQueuePriorities = &queuePriority
	};

	// Device Features 
	vk::PhysicalDeviceFeatures physicalDeviceFeatures;
	vk::StructureChain<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
		featureChain =
	{
		{},    // każda {} inicjalizuje jedną z pozycji w StructureChain
		{.shaderDrawParameters = true},
		{.dynamicRendering = true},
		{.extendedDynamicState = true }
	};

	auto& Vulkan13FeaturesToSet = featureChain.get<vk::PhysicalDeviceVulkan13Features>();
	Vulkan13FeaturesToSet.synchronization2 = vk::True;

	std::vector<const char*> requiredDeviceExtension = { vk::KHRSwapchainExtensionName };

	vk::DeviceCreateInfo logicalDeviceCreateInfo{
		.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(), 
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &deviceQueueCreateInfo,
		.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
		.ppEnabledExtensionNames = requiredDeviceExtension.data()
	};

	logicalDevice = vk::raii::Device(physicalDevice, logicalDeviceCreateInfo);
	graphicsQueue = vk::raii::Queue(logicalDevice, graphicsQueueIndex, 0);
}

std::vector<const char*> VkContext::GetRequiredInstanceExtensions()
{
	uint32_t glfwExtensionsCount = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
	std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);  // tu jest arytmetyka wskaźników -> przesuwamy wskaźnik o glfwExtensionCount

	return extensions;
}

bool VkContext::IsDeviceSuitable(vk::raii::PhysicalDevice const& InPhysicalDevice)
{
	auto deviceProperties = InPhysicalDevice.getProperties();
	auto deviceFeatures = InPhysicalDevice.getFeatures();
	if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && deviceFeatures.geometryShader)
	{
		return true;
	}

	return false;
}

vk::raii::PhysicalDevice* VkContext::ChoosePhysicalDeviceByScore(std::vector<vk::raii::PhysicalDevice>& InPhysicalDevices)
{
	// multimap pozwala na posiadanie w mapie kilku takich samych kluczy ( normalnie to jest niemożliwe )
	// generalnie klucze są posortowane
	std::multimap<int, vk::raii::PhysicalDevice*> candidates;   

	for (auto& physicalDevice : InPhysicalDevices)
	{
		auto deviceProperties = physicalDevice.getProperties();
		auto deviceFeatures = physicalDevice.getFeatures();

		std::cout << deviceProperties.deviceName << "\n";

		uint32_t score = 0;

		// discrette - czyli dedykowana karta jest bardziej wydajna niż zintegrowana
		if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		{
			score += 1000;
		}

		if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
		{
			score += 500;
		}

		auto chainPeoperties = physicalDevice.template getProperties2<vk::PhysicalDeviceProperties2,
			vk::PhysicalDeviceDriverProperties>();

		auto driverProperties = chainPeoperties.get<vk::PhysicalDeviceDriverProperties>();
		if (driverProperties.driverID == vk::DriverId::eQualcommProprietary)
		{
			score += 2000;
		}

		// Maksymalny rozmiar textur ( rozmiar textur może wpływać na jakość grafiki )
		score += deviceProperties.limits.maxImageDimension2D;

		// Zakładamy że aplikacja nie może działać bez geometry shaders
		if (!deviceFeatures.geometryShader)
		{
			continue;
		}

		candidates.insert(std::make_pair(score, &physicalDevice));
	}

	if (!candidates.empty() && candidates.rbegin()->first > 0)
	{
		return std::move(candidates.rbegin()->second);
	}
	else
	{
		throw std::runtime_error("failed to find suitable GPU!");
	}
}