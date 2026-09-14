#include "VkContext.h"
#include <iostream>
#include <map>


#define GLFW_INCLUDE_VULKAN // czyli w glfw3.h jest #if dedined(GLFW_INCLUDE_VULCAN) -> 
#include <GLFW/glfw3.h> 

#include "GLFWWindow/WindowGLFW.h"

void Vk_Context::CreateInstance()
{
	constexpr vk::ApplicationInfo appInfo
	{
		.pApplicationName = "Hello Triangle",
		.applicationVersion = VK_MAKE_VERSION(1,0,0),
		.pEngineName = "No engine",
		.engineVersion = VK_MAKE_VERSION(1,0,0),
		.apiVersion = vk::ApiVersion14
	};

	// Ok ten kod tu pozwolił nam sprawdzić czy wszystkie extensions są obsługiwane -> ale generalnie nie musimy go cały czas mieć
	// //
	// 
	// Generalnie z tymi extensions to chodzi o to, że glfw sprawdza na jakim systemie działa itp, i zwraca tablicę extensions
	// których potrzebuje od vulkana aby móc utworzyć powierzchnię Vulkan dla okna GLFW.
	// Extensions nie są to "pluginy" które się dodaje czy coś. Extensions są to części samego vulkana, które nie są domyślnie ładowane
	// - np wyświetlanie obrazu w oknie ( bo vulkan nie musi renderować w oknie ), czy funkcje specyficzne dla danego systemu operacyjnego.
	// ( zrobić fiszkę z extensions - co to są i po co się je ładuje )

	auto requiredGLFWExtensions = GetRequiredInstanceExtensions();  // glfwExtensionCount <- to jest out parameter, więc funkcja zwraca char*  +  zwraca wartość glfwExtensionCount
	// glfWExtensions jest pointerem ale pointerem na początek tablicy.

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

	//std::ranges::none_of(collection, predicate) -> zwraca true gdy żaden element kolekcji nie spełnia warunków predicate -> czyli jeśli żaden nie spełnia - to znaczy że żaden nie pasuje i wchodzimy w throw
	// Poniżej sprawdzamy czy wymagane przez glfw Extensions są wspierane przez danego vulkana
	for (uint32_t i = 0; i < requiredGLFWExtensions.size(); i++)
	{   // none_of jako całość będzie true, jeśli wszystkie sprawdzenia nie spełnią warunku strcmr() zwraca 0 jeśli dwa napisy są równe !!!
		//  czyli none_of - jeśli żaden nie zwróci true, to none_of jest true - i wchodzimy w wyjątek, jeśli chodź jeden zwróci true, to none_of jest false !
		if (std::ranges::none_of(extensionProperties, [glfwExtension = requiredGLFWExtensions[i]](auto const& extensionProperty)
			{
				return strcmp(extensionProperty.extensionName, glfwExtension) == 0;
			}))
		{
			throw std::runtime_error("Required GLFW extension not supported: " + std::string(requiredGLFWExtensions[i]));
		}
	}
	// -> i do instanceInfo przekazywana jest informacja które extensions Vulkana są potrzebne


	//auto requiredExtensions = getRequiredInstanceExtensions();

	// Teraz sprawdzamy potrzebne validation layers
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

	// std::ranges::find_if zwraca iterator na pierwszy element który nie jest obsługiwany. Jeśli przeiterujemy wszystkie elementy z 
	// requiredLayers, to oznacza że wszystkie są obsługiwane, a iterator powinien wskazywać na requiredLayers.end()
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
		// przekazujemy za pomocą .data() wskaźnik na pierwszy element tablicy ( no i mamy size tablicy, więc Vulkan może iterować )
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

void Vk_Context::CreateSurface(WindowGLFW* InWindow)
{
	VkSurfaceKHR _surface;  
	if (glfwCreateWindowSurface(*instance, InWindow->window, nullptr, &_surface) != 0)
	{
		throw std::runtime_error("Failed to create window surface");
	}
	surface = vk::raii::SurfaceKHR(instance, _surface); 
}

void Vk_Context::PickPhysicalDevice()
{
	auto physicalDevices = instance.enumeratePhysicalDevices();

	if (physicalDevices.empty())
	{
		throw std::runtime_error("failed to find GPU with Vulkan support!");
	}

	for (auto& physicalDevice : physicalDevices)
	{
		//std::cout <<  << "\n";
		auto deviceProperties = physicalDevice.getProperties(); // getProperties() zwraca nam podstawowe dane device - nzawe, typ, obsługiwaną wersję Vulkan
		auto deviceFeatures = physicalDevice.getFeatures(); //  getFeatures() zwraca bardziej szczegółowe informacje, np czy obsługuje kompresje textur, 64-bitowe floaty czy np multi viewport rendering
		IsDeviceSuitable(physicalDevice);
	}

	physicalDevice = *ChoosePhysicalDeviceByScore(physicalDevices);

	// Tak możemy sprawdzić czy dana karta obsługuje wersję Vulkana >= 1.3
	bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

	// Poniżej sprawdzamy np czy karta obsługuje Queue graphics command ( bo różne queue są używane do wspierania różnych rodzajów command
	std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

	bool supportsGraphics = std::ranges::any_of(queueFamilies, [](const auto& queueFamily)
		{
			return !!(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics);
		});

	bool supportsCompute = std::ranges::any_of(queueFamilies, [](const auto& queueFamily)
		{
			return !!(queueFamily.queueFlags & vk::QueueFlagBits::eCompute);
		});

	// Tu poniżej sprawdzamy, które z extensions kótre będziemy potrzebować, są obsługiwane przez kartę ( wcześniej sprawdzaliśmy które z extensions
	// wymagane przez glfw są obsługiwane przez instancję vulkana )  ( ręcznie wpisujemy te które potrzebujemy w requiredExtensions )
	std::vector<const char*> requiredExtensions = { vk::KHRSwapchainExtensionName };
	auto availableDeviceExtension = physicalDevice.enumerateDeviceExtensionProperties();

	// Ok -> to co chcemy sprawdzić, to czy wszystkie z requiredExtensions są dostępne w availableDeviceExtansion
	bool areAllExtensionsSupportedByGPU = std::ranges::all_of(requiredExtensions, [&availableDeviceExtension](const auto& requiredExtension)
		{
			return std::ranges::any_of(availableDeviceExtension, [&requiredExtension](const auto& availableExtension)
				{
					return strcmp(availableExtension.extensionName, requiredExtension) == 0;
				});
		});
	// Powyżej jest tak - all_of - czyli bierzemy wszystkie requiredExtensions i iterujemy po każdym po kolei - dla każdego z kolei jako refka przyjmujemy availableExtension
	// wywołujemy lambdę, i w tej lambdzie bierzemy extension które chcemy sprawdzić i kolekcję dostępnych extensions, którą wzięliśmy jako refkę
	// i dla każdego extension wywołujemy ::any_of - żeby sprawdzić czy którekolwiek z availableExtensions ma taką samą nazwę jak nasze requiredExtension

	// A tu sprawdzamy dostępność features - czyli czy karta obsługuje konkretne funkcje
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
		//PhysicalDevice -> fallback to nullptr
		physicalDevice = nullptr;
		std::runtime_error("Physical Device pick error");
	}
}

void Vk_Context::CreateLogicalDevice()
{
	// DeviceQueueCreateInfo 
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

	// Musimy dowiedzieć się jaki queue która nas interesuje ma index, i ten index przekazać do DeviceQueueCreateInfo

	// Tak zamiast std::ranges - bo to czego nam potrzeba to tak na prawdę index
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		// poza tym że nasza queue musi wspierać eGraphics, musi ona również wspierać prezentację na surface - dodajemy więc sprawdzenie
		// -> i w tym miejscu musimy już mieć gotowe surface bo getSurfaceSupportKHR przyjmuje index queue i *surface
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

	float queuePriority = 0.5f;  // <- nawet jeśli mamy jedną kolejkę to musimy utworzyć dla niej queue priority - zakres 0.0 - 1.0
	// gdybym tworzył więcej niż jedną kolejkę - musiałbym podać np 2 queue priority 
	// -> std::array<float,2> queuePriorities {0.5f, 1.0f}; i później queuePriorities.data()  ( domyślam się że analogicznie dla famili index i queueCount )
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
													.queueFamilyIndex = graphicsQueueIndex,
													.queueCount = 1,
													.pQueuePriorities = &queuePriority
	};

	// Device Features 
	vk::PhysicalDeviceFeatures physicalDeviceFeatures;

	// Generalnie structure chain nie jest powszechną praktyką C++ową - stosuje się ją po to aby zachować kompatybilność przez lata.
	//Ok - teraz czaję - structure chain jest tworzony po to, aby możliwe było przekazanie tej struktury dalej - tam gdzie kod jest obsługiwany 
	// przez C. Generalnie możnaby to zrobić jako np std::vector - ale C go nie obsługuje. Też nie wiadomo jaką tablicę by z tego zrobić - 
	// więc tworzony jest po prostu structure chain - że każdy vk::Cośtam będzie dostawał pNext - wskaźnik na kolejną strukturę - i tak zostają one
	// przekazane do kodu który działa w C -> do logical device przekazujemy wskaźnik na pierwszą strukturę z łańcuchu, a ona i kolejne, posiadają
	// wskaźnik na kolejne struktury które chcemy aby były przekazane.
	// -> wcześniej sprawdzaliśmy czy karta wspiera te features - teraz oznaczamy że będziemy ich używać
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

	// Ok, a żeby włączyć depthClamp, muszę dostać się do niego osobno, przez feature chain
	// depthClamp:

	//featureChain.get<vk::PhysicalDeviceFeatures2>().features.depthClamp = true;
	// Czyli jeszcze raz - w physical device - sprawdzam czy karta coś obsługuje - w logical device łaczam tany feature że będę chciał go użyć

	auto& Vulkan13FeaturesToSet = featureChain.get<vk::PhysicalDeviceVulkan13Features>();
	Vulkan13FeaturesToSet.synchronization2 = vk::True;

	// Device Extensions
	// Wcześniej sprawdzaliśmy czy te extensions są wspierane przez kartę, przy wybieraniu karty - teraz oznaczamy że będziemy ich używać
	std::vector<const char*> requiredDeviceExtension = { vk::KHRSwapchainExtensionName };

	vk::DeviceCreateInfo logicalDeviceCreateInfo{
		.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),   // do pNext podłaczamy featureChain - bierzemy wskaźnik na pierwszy element chaina, i on łączy z kolejnymi
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &deviceQueueCreateInfo,
		.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
		.ppEnabledExtensionNames = requiredDeviceExtension.data()
	};

	logicalDevice = vk::raii::Device(physicalDevice, logicalDeviceCreateInfo);
	// Tworzenie handlera dla queue ( zarequestowane queue jest tworzone automatycznie wraz z tworzeniem logical device - potrzebujemy jednak jakiegoś uchwytu do niego )
	graphicsQueue = vk::raii::Queue(logicalDevice, graphicsQueueIndex, 0);
}

std::vector<const char*> Vk_Context::GetRequiredInstanceExtensions()
{
	uint32_t glfwExtensionsCount = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
	std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);  // tu jest arytmetyka wskaźników -> przesuwamy wskaźnik o glfwExtensionCount

	return extensions;
}

bool Vk_Context::IsDeviceSuitable(vk::raii::PhysicalDevice const& InPhysicalDevice)
{
	auto deviceProperties = InPhysicalDevice.getProperties();
	auto deviceFeatures = InPhysicalDevice.getFeatures();
	if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && deviceFeatures.geometryShader)
	{
		return true;
	}

	return false;
}

vk::raii::PhysicalDevice* Vk_Context::ChoosePhysicalDeviceByScore(std::vector<vk::raii::PhysicalDevice>& InPhysicalDevices)
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