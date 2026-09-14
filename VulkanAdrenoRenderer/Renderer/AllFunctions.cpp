#include "HelloTriangleApp.h"
#include <iostream>
#include <map>

#include <fstream> // żeby odczytywać pliki

void HelloTriangleApp::createInstance()
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

	auto requiredGLFWExtensions = getRequiredInstanceExtensions();  // glfwExtensionCount <- to jest out parameter, więc funkcja zwraca char*  +  zwraca wartość glfwExtensionCount
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

std::vector<const char*> HelloTriangleApp::getRequiredInstanceExtensions()
{
	uint32_t glfwExtensionsCount = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

	std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);  // tu jest arytmetyka wskaźników -> przesuwamy wskaźnik o glfwExtensionCount

	return extensions;
}
// ## Instance End ##

// ## Physical Device 
void HelloTriangleApp::pickPhysicalDevice()
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
		isDeviceSuitable(physicalDevice);

	}

	physicalDevice = *choosePhysicalDeviceByScore(physicalDevices);

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

	// YEAH !! jak mam qualcomm propertiaryguygu driver to obsługuje wszystko !! -> i znów zajebiście że się nie poddałem!


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

bool HelloTriangleApp::isDeviceSuitable(vk::raii::PhysicalDevice const& InPhysicalDevice)
{
	auto deviceProperties = InPhysicalDevice.getProperties();
	auto deviceFeatures = InPhysicalDevice.getFeatures();

	// sprawdzamy czy karta jest typu vk::PhysicalDeviceType::eDiscreteGpu i czy obsługuje geometryShader ( czyli deviceFeatures.geometryShader jest true (1) )
	if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && deviceFeatures.geometryShader)
	{
		return true;
	}

	return false;
}

vk::raii::PhysicalDevice* HelloTriangleApp::choosePhysicalDeviceByScore(std::vector<vk::raii::PhysicalDevice>& InPhysicalDevices)
{
	// multimap pozwala na posiadanie w mapie kilku takich samych kluczy ( normalnie to jest niemożliwe )
	// generalnie klucze są posortowane
	std::multimap<int, vk::raii::PhysicalDevice*> candidates;   // Ahh ! no tak - wywala, bo candidates przechowuje wartość !
	// a wraz z wyjściem z tej funkcji multimap candidates ginie - i giną obiekty ! - czyli mapa musi albo przechowywać wskaźniki, albo musimy przenieść obiekt z mapy
	// ( fajnie że to sam rozwiązałem, zamiast pytać GPT !) 

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
			score += 1000;
		}

		auto chainPeoperties = physicalDevice.template getProperties2<vk::PhysicalDeviceProperties2,
			vk::PhysicalDeviceDriverProperties>();

		auto driverProperties = chainPeoperties.get<vk::PhysicalDeviceDriverProperties>();

		if (driverProperties.driverID == vk::DriverId::eQualcommProprietary)
		{
			score += 1000;
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

void HelloTriangleApp::createLogicalDevice()
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

// ## Surface ## 
void HelloTriangleApp::createSurface()
{// tu mamy kod glfw który automatycznie tworzy dla nas odpowiednią surface dla okna, w zależności od systemu operacyjnego
	// Utworzenie surface wymaga włączonych extensions - które wcześniej glfw zwróciło jako wymagane ( co już zostało zrobione w InstanceCreateInfo )
	VkSurfaceKHR _surface;   // <- ten obiekt jest typu C. dalej ten kod: vk::raii::SurfaceKHR(instance, _surface); wsadza go w C++ wrapper

	// Ok - czyli glfwCreateWindowSurface jeśli się powiedzie zwraca 0 ( jak funkcja main, return 0 )
	if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0)
	{
		throw std::runtime_error("Failed to create window surface");
	}

	surface = vk::raii::SurfaceKHR(instance, _surface);  // zniszczeniem SurfaceKHR zajmuje się już raii
}

// ## Swap Chain
void HelloTriangleApp::createSwapChain()
{
	// Czyli pewnie też tworzymy SwapChainCreateInfo

	// to nam zwraca dostępne dla surface formaty ( pixel format, color space )
	std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(*surface);

	// dostępne present modes ( presentation mode - czyli warunki/sposoby "swapowania" obrazów na ekranie )
	// PresentationMode jest prawdopodobnie najważniejszym settingsem dla SwapChain
	std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);

	// Niżej będą funkcje, które sprawdzają czy wybrane przez nas ustawienia są dostępne, a jeśli nie to szuka innych najlepszych dostępnych
	swapChainSurfaceFormat = chooseSwapChainSurfaceFormat(availableFormats);
	vk::PresentModeKHR   presentMode = chooseSwapPresentMode(availablePresentModes);

	// surface capabilities zwraca nam podstawowe możliwości powierzchni (max ilość obrazów w swap chain i ich max wymiary)
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
	swapChainExtent = chooseSwapChainExtent(surfaceCapabilities);

	// generalnie jest określone minimum swap chain images dla danej implementacji
	surfaceCapabilities.minImageCount;  // warto jednak mieć więcej images we swap chain niż minimum - jednocześnie musimy mieć na uwadze aby nie przekroczyć maksymalnej liczby obrazów we swap chainie
	uint32_t swapChainImageCount = chooseSwapChainMinImageCount(surfaceCapabilities);

	std::cout << "SwapChainImageCount =  " << swapChainImageCount << "\n";

	vk::SwapchainCreateInfoKHR swapChainCreateInfo{
		.surface = *surface,
		.minImageCount = swapChainImageCount,
		.imageFormat = swapChainSurfaceFormat.format,
		.imageColorSpace = swapChainSurfaceFormat.colorSpace,
		.imageExtent = swapChainExtent,
		.imageArrayLayers = 1,  // <- zawsze dajemy jeden, chyba że renderujemy dla VR ("stereoscopic 3D application")
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = surfaceCapabilities.currentTransform,   // <- w sensie że obraz może być np obrócony o 90 stopni
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = presentMode,
		.clipped = true,
		.oldSwapchain = nullptr   // <- przy zamianie swap chain, jeśli chcemy renderować w czasie kiedy wykonywana jest zmiana swap
		// chaina, możemy podać aktualny swapChain jako .oldSwapChain ( ok ale w tym momencie nie do końca to kminie )
	};

	swapChain = vk::raii::SwapchainKHR(logicalDevice, swapChainCreateInfo);
	swapChainImages = swapChain.getImages();
}

vk::SurfaceFormatKHR HelloTriangleApp::chooseSwapChainSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats)
{
	const auto formatIT = std::ranges::find_if(availableFormats, [](const vk::SurfaceFormatKHR& availableFormat) {
		return (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear);
		});
	// Jeżeli iterator == availableFormats.end() to znaczy że dotarł do końca - i nic nie znalazł. 
	// jeżeli znalazł to zatrzymał się na tym elemencie. jak zrobimy *formatIT to pobierzemy element na którym zatrzymał się iterator
	return formatIT != availableFormats.end() ? *formatIT : availableFormats[0];
}

vk::PresentModeKHR HelloTriangleApp::chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes)
{
	// tylko eFifo jest zawsze gwarantowane  -> eFifo jest np dobre dla mobile - zmniejsza zużycie energii
	// vk::PresentModeKHR::eFifo;

	// my teraz szukamy eMailbox - jest dobre jeżeli zużycie energii nie jest problemem. Pozwala na niskie opóźnienia i unikanie tearingu
	// eMailbox - określane też jest jako triple buffering
	// Tu robimy asercję że eFifo jest zawsze dostępne
	assert(std::ranges::any_of(availablePresentModes, [](const vk::PresentModeKHR& presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));

	// find_if -> zwraca iterator DO konkretnego elementu. a my potrzebujemy tylko odpowiedzi czy element którego szukamy jest w ogóle w
	// liście, więc możemy zastosować bespośrednio any_of   -> jeśli any_of zwraca true to mamy eMailbox, jeśli false no to eFifo
	return std::ranges::any_of(availablePresentModes, [](const vk::PresentModeKHR& presentMode) { return presentMode == vk::PresentModeKHR::eMailbox; })
		? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
}

// Extent2d jest to rozmiar obrazów we swap chain. Prawie zawsze jest to po prostu rozmiar okna 
vk::Extent2D HelloTriangleApp::chooseSwapChainExtent(vk::SurfaceCapabilitiesKHR const& capabilities)
{
	// To jest normalny przypadek - jako Extent2D zwracamy rozmiar okna
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}

	// Niektóre menedżery okien pozwalają na zmianę rozdzielczości, co sygnalizują ustawiając rozmiar SurfaceCapabilitiesKHR na std::numeric_limits<uint32_t>::max()
	// w takim przypadku pobieramy faktyczny rozmiaz z frame buffera glfw -
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	// jak pobierzemy fasktyczne rozmiary z frame buffera, to clampujemy je - jeśli np rozmiar jest mniejszy niż minImageExtent - to zwracamy minImageExtend
	// jeśli większy niż maxImageExtent - to analogicznie zwracamy maxImageExtent
	return {
		std::clamp<uint32_t>(width,capabilities.minImageExtent.width,capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(width,capabilities.minImageExtent.height,capabilities.maxImageExtent.height)
	};
}

uint32_t HelloTriangleApp::chooseSwapChainMinImageCount(vk::SurfaceCapabilitiesKHR const& capabilities)
{
	// 3u to po prostu 3 ze wskazaniem ze unsigned int
	auto ImageCount = std::max(3u, capabilities.minImageCount);

	// Jeżeli capabilities.maxImageCount == 0 to oznacza że nie ma maximum dla ilości images
	// Tu poniżej po prostu upewniamy się że maxImageCount nie jest bez limitu i jednocześnie nie jest mniejszy niż ImageCount który ustawiliśmuy
	// -> jeśli jest, to używamy maxImageCount, a nie ImageCount który ustawiliśmy 
	if ((0 < capabilities.maxImageCount) && (capabilities.maxImageCount < ImageCount))
	{
		ImageCount = capabilities.maxImageCount;
	}

	return ImageCount;
}

void HelloTriangleApp::cleanupSwapChain()
{
	swapChainImageViews.clear();
	swapChain = nullptr;
}

void HelloTriangleApp::createImageViews()
{
	assert(swapChainImageViews.empty());

	vk::ImageViewCreateInfo imageViewCreateInfo{
		.viewType = vk::ImageViewType::e2D,
		.format = swapChainSurfaceFormat.format,
		.subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,  // <- aspectMask określa właściwe przeznaczenie obrazu, tu, że będziemy renderować do niego kolor a nie np depth
							  .baseMipLevel = 0,
							  .levelCount = 1,
							  .baseArrayLayer = 0,
							  .layerCount = 1}
	};

	// ImageView pozwala również na zarzadzanie kanałami RGBA - możemy wymusić żeby zrobić np monochromatyczny obraz z kanału R
	// -> RRRA    -> jeśli ednak zastosujemy vk::ComponentSwizzle::eIdentity - to do danego kanału, zostanie przekazany jego defaultowy kanał
	imageViewCreateInfo.components = {
		vk::ComponentSwizzle::eIdentity,vk::ComponentSwizzle::eIdentity,vk::ComponentSwizzle::eIdentity,vk::ComponentSwizzle::eIdentity
	};

	//If you were working on a stereographic 3D application, then you would create a swap chain with multiple layers.
	//You could then create multiple image views for each image representing the views for the left and right eyes by 
	//accessing different layers.

	// Co do zasady, można oczekiwać że większość kart graficznych będzie mogła używać max 16 ImageViews. Jednak inne egzotyczne rozwiązania,
	// mogą ich potrzebować wiele więcej ( np CAVE displays )

	// W momencie utworzenia swapChain wypełniona została lista vk::Image   - teraz iterujemy po wszystkich Image w swapChainImages i dla każdego
	// tworzymy ImageView i dodajemy do naszej listy  ( emplace_back(device,imageViweCreateInfo) tworzy nowy obiekt ImageView

	for (auto& image : swapChainImages)
	{
		imageViewCreateInfo.image = image;
		// zmieniamy imageViewCreateInfo.image dla każdego Image i z tak zmienionymCreate info ( przypisanym do danego image ), tworzymy ImageView
		swapChainImageViews.emplace_back(logicalDevice, imageViewCreateInfo);
	}

}


// ## Graphics Pipeline ##    -
void HelloTriangleApp::createGraphicsPipeline()
{
	std::vector<uint32_t> shaderCode = readFile("slang.spv");

	// Sprawdzenie że alignment będzie poprawny - ale zrobić żeby przechowywać jednak bity jako uint32_t
	//assert(shaderCode.size() % sizeof(uint32_t) == 0);

	vk::raii::ShaderModule shaderModule = createShaderModule(shaderCode);

	std::cout << "Byte:  " << sizeof(char) << "   uint32_t:  " << sizeof(uint32_t) << "\n";

	// Naszego shader module przypisujemy później do konkretnego pipeline stage - i podajemy nazwę głównej funkcji, którą ten pipeline stage
	// wywołuje

	vk::PipelineShaderStageCreateInfo vertexShaderCreationInfo
	{
		.stage = vk::ShaderStageFlagBits::eVertex,
		.module = shaderModule,
		.pName = "vertMain"  // <- i to jest nazwa funkcji "wejściowej" w shaderze dla tego stepu pipeline
	};

	vk::PipelineShaderStageCreateInfo fragmentShaderCreationInfo
	{
		.stage = vk::ShaderStageFlagBits::eFragment,
		.module = shaderModule,
		.pName = "fragMain"
	};

	vk::PipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreationInfo,fragmentShaderCreationInfo };


	// Niektóre dane moga być przekazywane dynamicznie, co oznacza że należy ja przekonać do PipelineDynamicState ( i nie trzeba przebudowywać całego
	// pipeline przy zmianie tych danych - np rozmiar okna )
	std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

	vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data()
	};

	// wertex Input Create Info -> służy do opisu formatu w jakim przekazywane będa vertexy do vertex shadera
	// są dwie opcje przekazywania - Bindings i Attibute descriptions.  W tym momencie zahardkodowaliśmy vertexy w pixel shader
	// więc tworzymy po prostu
	vk::PipelineVertexInputStateCreateInfo  vertexInputInfo;

	// InputAssembly określa: jaki rodzaj geometrii będzie rysowany z vertexów i czy primitive restart jest enabled czy nie
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{ .topology = vk::PrimitiveTopology::eTriangleList };


	// Viewport -  określa region framebuffera do którego będzie renderowany output ( praktycznie zawsze ma rozmiar (0,0) - (width,height)
	// ostatnie 0.0f, 1.0f - to minDepth i maxDepth ( czyli to co się pojawia w depth buffer - standardowo zakres 0.0 do 1.0 )
	vk::Viewport viewport{ 0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f };

	// Scissors - określają wycinek który ma być renderowany - pixele które nie mieszczą się w ramach rectangla ze scissorsów są po prostu 
	// ignorowane - jeśli chcemy renderować cały Viewport - to ustawiamy scissorsy tak aby pokrywały caly viewport
	vk::Rect2D scissor{ vk::Offset2D{0,0}, swapChainExtent };
	// Viewport i scissors mogą być używane z dynamic state, a więc mogą być ustawiane dynamicznie.

	// Później ilość viewportów i scissorów i viewporta i scissorsa przypisujemy do PipelineViewportStateCreateInfo
	vk::PipelineViewportStateCreateInfo viewportState{
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor
	};

	// Rasterizer !
	// Rasterizer przekształca geometrie z vertex shader we fragmenty które będą pokolorowane przez fragment shader.
	// Robi depth test, face culling i scissors test. Można też ustawić czy zwraca fragmenty któe pokrywają cały polygon, czy tylko krawędzie (renderowanie wireframe)
	vk::PipelineRasterizationStateCreateInfo rasteriser{
		.depthClampEnable = false,  // <-, depthClamp jest użyteczne dla generowania shadowMaps. Użycie tego wymaga włączenia GPU feature
		.rasterizerDiscardEnable = false,  // <- jeśli true to powoduje że geometria nie przechodzi przez rasterizer
		.polygonMode = vk::PolygonMode::eFill,  // <- użycie innego mode niż eFill wymaga włączenia GPU feature
		.cullMode = vk::CullModeFlagBits::eBack, // <- można włączyć, wyłączyć, albo ustawić na back/front
		.frontFace = vk::FrontFace::eClockwise,  // <- wskazuje która kolejność wierzchołków będzie ustalać czy jest front czy backface
		.depthBiasEnable = vk::False,  // <- nie wiem po chuja vk::False - może być zwykłe false
		.lineWidth = 1.0f  // <- ustawia grubość linii w kontekście ilości fragmentów ( zwiększenie wielkości lineWidth ponad 1.0 wymaga włączenia wideLines GPU feature )
		// <- lineWidth jest używane gdy renderujemy wireframe. Jak renderujemy zwykłe eFill to właściwie nie ma znaczenia
	};


	// Multisampling - czyli forma antyaliasingu  - wymaga włączenia featura na GPU - puki co trzymamy wyłączony
	vk::PipelineMultisampleStateCreateInfo multisampling{ .rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False };


	// Color blenging - po tym jak fragment shader zwróci kolor, ten kolor musi być w jakiś sposób połaczony z kolorem który jest we framebuffer ( właściwie to w render targecie. Sam frame buffer nie przechowuje koloru - Kolor przechowuje Image)
	// - można albo zrobić Mix, albo bitwise operation

	// vk::PipelineColorBlendAttachementState - przechowuje konfigurację per framebuffer
	vk::PipelineColorBlendAttachmentState colorBlendAttachement{
		.blendEnable = vk::False,

		// a jeśli zrobimy blendEnalbe = true, to musimy ustawić inne parametry:
		/*
		.blendEnable = vk::True,
		.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
		.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
		.colorBlendOp        = vk::BlendOp::eAdd,
		.srcAlphaBlendFactor = vk::BlendFactor::eOne,
		.dstAlphaBlendFactor = vk::BlendFactor::eZero,
		.alphaBlendOp        = vk::BlendOp::eAdd,
		*/

		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	};

	// vk::PipelineColorBlendStateCreateInfo  <- zawiera globalne ustawienia color blendingu -> przekazujemy do niego też 
	// utworony wcześniej PipelineColorBlendAttachmentState  ( można przekazać wiele )
	// Generalnie z blendingiem kolorów chodzi o to jak np kolor z nowego renderu będzie mieszany ze starym. 
	// Najczęstszym blendingiem jest alpha blending np:    
	// finaColor.rgb =  alpha * newColor + (1-alpha)* oldColor
	// finalColor.a  =  newAlpha.a

	vk::PipelineColorBlendStateCreateInfo colorBlending{
		.logicOpEnable = vk::False,
		.logicOp = vk::LogicOp::eCopy,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachement
	};


	// Pipeline Layout -> pozwala na przekazywanie dynamicznych wartości do shaderów -> Puki co nie będziemy używać, ale i tak musimy utworzyć
	// pusty obiekt i pole w klasie HelloTriangleApp
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		.setLayoutCount = 0,
		.pushConstantRangeCount = 0
	};

	pipelineLayout = vk::raii::PipelineLayout(logicalDevice, pipelineLayoutCreateInfo);

	// Pipeline
	vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo
	{
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &swapChainSurfaceFormat.format
	};

	vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain
	{
		vk::GraphicsPipelineCreateInfo{
			.stageCount = 2,
			.pStages = shaderStages,
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasteriser,
			.pMultisampleState = &multisampling,
			.pColorBlendState = &colorBlending,
			.pDynamicState = &dynamicStateCreateInfo,
			.layout = pipelineLayout,
			.renderPass = nullptr
		},

		vk::PipelineRenderingCreateInfo
		{
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &swapChainSurfaceFormat.format
		}
	};

	pipeline = vk::raii::Pipeline(logicalDevice, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());

}

std::vector<uint32_t> HelloTriangleApp::readFile(const std::string& filename)
{
	// std::ios::ate powoduje że otwieramy plik i od razu przechodzimy na jego koniec - pozwala to od razu sprawdzić rozmiar pliku.
	// a binary - no to że plik czytany binarnie.
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file");
	}

	// tellg() odczytuje aktualną pozcję "kursora odczytu". Skoro nasz vector przyjmuje <char> czyli bajty, a pozycja odczytu jest podawana w bajtach
	// to pozycja odczytu jest rozmiarem naszego vectora  ( tu zmiana, bo zmieniliśmy char na uint32_t, więc musimy podzielić ilość bajtów
	// przez rozmiar uint32_t które są potrzebne do 
	std::vector<uint32_t> buffer(file.tellg() / sizeof(uint32_t));

	file.seekg(0, std::ios::beg);

	// read zapisuje do 1-arg buffer.data() zwraca wskaźnik na początek bufora, drugi argument to rozmiar ile ma czytać - nasz bufor ma rozmiar pliku
	// i tyle ma czytać ( castujemy rozmiar na std::streamsize)   // reinterpret_cast<char*> bo read obsługuje tylko char.
	file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size() * sizeof(uint32_t)));

	file.close();

	return buffer;
}

vk::raii::ShaderModule HelloTriangleApp::createShaderModule(const std::vector<uint32_t>& shaderBytes)
{
	vk::ShaderModuleCreateInfo createInfo
	{
		.codeSize = shaderBytes.size() * sizeof(uint32_t),
		// sla shaderModule wskaźnik który przekazujemy powinien być wskaźnikiem na uint32_t a nie na char, więc go reinterpret castujemy
		.pCode = shaderBytes.data()
		// robiąc reinterpret cast, musimy być pewni że dane spałniają wymagania alignmentu ( szczęśliwie std::vector zajmuje się tym )
	};

	// UWAGA - tu jest jakaś nieścisłość z alignmentem - bo vector zapewnia alignment ale dla char, a nie uint32_t - czyli lepiej by było przechowywać
	// kod bitowy w formie uint32_t

	return vk::raii::ShaderModule{ logicalDevice, createInfo };
}

void HelloTriangleApp::createCommandPool()
{
	vk::CommandPoolCreateInfo commandPoolCreateInfo
	{
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, // eTransient - kiedy rzadko będą nagrywane nowe commandsy
		.queueFamilyIndex = graphicsQueueIndex
	};

	// Command Pool - alokujje i zarządza command Bufferami
	// Command buffer - obiekt który zawiera serię instrukcji dla GPU
	// queue - obiekt który przyjmuje command buffer do wykonania

	commandPool = vk::raii::CommandPool(logicalDevice, commandPoolCreateInfo);
}

void HelloTriangleApp::createCommandBuffers()
{
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo{
		.commandPool = commandPool,
		.level = vk::CommandBufferLevel::ePrimary, // <- jesli jest primary może być przekazany do queue ale nie może być wywołany przez inny command buffer ( jeśli secondary to inny command buffer może go wywołać )
		.commandBufferCount = MAX_FRAMES_IN_FLIGHT
	};

	// Ok ten kod poniżej automatycznie tworzy nam vector z commandBufferami. wcześniej po prostu pobieraliśmy .first() czyli pierwszego z nich
	commandBuffers = vk::raii::CommandBuffers(logicalDevice, commandBufferAllocateInfo); // CommandBuffers tworzy std::vector<vk::raii::CommandBuffer> ale tu używamy tylko jednego i tylko jego zapisujemy jako pole obiektu

}

void HelloTriangleApp::recordCommandBuffer(uint32_t imageIndex)
{

	//vk::CommandBufferBeginInfo beginInfo{
	//	.flags = 
	//		vk::CommandBufferUsageFlagBits::eOneTimeSubmit 
	//		//vk::CommandBufferUsageFlagBits::eRenderPassContinue 
	//		//vk::CommandBufferUsageFlagBits::eSimultaneousUse
	//		
	//};
	// 
	// commandBuffer.begin(beginInfo); <- tak na prawdę nie potrzebujemy teraz żadnej z tych flag - i beginInfo nie jest nam potrzebne teraz
	auto& commandBuffer = commandBuffers[frameIndex];

	commandBuffer.begin({}); // rozpoczynamy recording <- jesli command buffer został nagrany, to kolejne wywołanie begin resetuje go. Nie jest możliwe dodawanie instrukcji do istniejącego command buffera

	// Przed renderowaniem, przetransformować swap chain image do vk::ImageLayout::eColorAttachmentOptimal
	transition_image_layout(
		imageIndex,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput
	);

	// Po zrobionej tranzycji do eColorAttachmentOptimal tworzymy color attachment
	vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	vk::RenderingAttachmentInfo attachmentInfo = {
		.imageView = swapChainImageViews[imageIndex],
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,    // <- co zrobić z obrazem przed renderowaniem
		.storeOp = vk::AttachmentStoreOp::eStore,  // <- co zrobić z obrazem po renderowaniu  ( store czyli zachowujemy do późńiejszego użycia )
		.clearValue = clearColor
	};

	// rendering info
	vk::RenderingInfo renderingInfo = {
		.renderArea = {.offset = {0,0}, .extent = swapChainExtent},  // <- czyli pokrywamy cały obraz
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachmentInfo
	};

	// Rozpoczynanie renderowania  ( przyjmuje rendering info jako argument)
	commandBuffer.beginRendering(renderingInfo);

	// Bindowanie pipeline       pipeline może być eGraphics albo eCompute
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);

	// Jako że viewport i scissors zrobiliśmy jako dynamic, musimy je teraz ustawić
	commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
	commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));

	commandBuffer.draw(
		3,  // <- vertex count  ( o bo mamy 3 w trójkącie )
		1,  // <- instance count ( używane do instanced rendering ) 
		0,  // first vertex   - definiuje the lowest value of SV_VertexID
		0   // first instance - definiuje the lowest value of SV_InstanceID
	);

	commandBuffer.endRendering();

	// Po wyrenderowaniu musimy przekształcić Image layout do vk::ImageLayout::ePresentSrcKHR <- zeby nadawało się do zaprezentowania na screenie
	transition_image_layout(
		imageIndex,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		{},
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eBottomOfPipe
	);

	commandBuffer.end();
}

void HelloTriangleApp::transition_image_layout(uint32_t imageIndex, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags2 src_access_mask, vk::AccessFlags2 dst_access_mask, vk::PipelineStageFlags2 src_stage_mask, vk::PipelineStageFlags2 dst_stage_mask)
{
	vk::ImageMemoryBarrier2 barrier =    // barrier w vulkanie jest to jawne polecenie synchronizacji - oznacza że operacje przed tym punktem muszą
	{									 // osiągnąć określony etap
										 // w uproszczeniu - bariera ustala że najpierw musi się zakończyć jakaś operacja na obiekcie, zanim będzie mogła zacząć się inna
		.srcStageMask = src_stage_mask,
		.srcAccessMask = src_access_mask,

		.dstStageMask = dst_stage_mask,
		.dstAccessMask = dst_access_mask,

		.oldLayout = oldLayout,
		.newLayout = newLayout,

		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,

		.image = swapChainImages[imageIndex],

		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	vk::DependencyInfo dependency_info = {
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier
	};

	commandBuffers[frameIndex].pipelineBarrier2(dependency_info);
}

void HelloTriangleApp::drawFrame()
{ // Zanim zaczniemy rysować klatkę, czekamy na fence - fence blokuje CPU - - draw frame jest wywoływane w pętli, więc chcemy żeby zaczekało kiedy
  // na pewno zakończy się renderować                          // vk::True wskazuje że czekamy na wszystkie fences ( w tym przypadku to bez znaczenia bo i tak jest jeden ) ( funkcja może czekać aż wszystkie fences będą signaled, albo jakikolwiek )

	// Uwaga teraz drawFences, presentCompleteSemaphores i commandBuffers zależą od frameIndex a 
	// renderFinishedSemaphores zależy od imageIndex

	auto fenceResult = logicalDevice.waitForFences(*drawFences[frameIndex], vk::True, UINT64_MAX);

	if (fenceResult != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait for fence");
	}

	//logicalDevice.resetFences(*drawFences[frameIndex]);  <- tu wcześniej było reset fences, ale musimy przeniesć je "po" return ( recreateSwapChain(); )

	// Teraz pobieramy wolny Image ze swapChain  -> dostajemy index Image którego możemy użyć ( a result to vk::Result )
	auto [result, imageIndex] = swapChain.acquireNextImage(UINT64_MAX, *getImageCompleteSemaphores[frameIndex], nullptr);

	if (result == vk::Result::eErrorOutOfDateKHR || frameBufferResized)
	{
		recreateSwapChain();
		return;
	}
	if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
	{
		assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
		throw std::runtime_error("Failed to acquire swap chain image");
	}

	logicalDevice.resetFences(*drawFences[frameIndex]); // <- przeniesienie tutaj wynika z tego, że jak mamy logicalDevice.waitForFences(*drawFences[frameIndex] to czekamy aż fence zostanie zasygnalizowany że
	// "można robić". Jeśli zresetujemy fence i wyjdziemy, to nie wywoła się nigdy funkcja graphicsQueue.submit(submitInfo, *drawFences[frameIndex]);, a własnie jej wykonanie gwarantuje "zasygnalizowanie" fence
	// - waitForFences(..) czeka aż fence będzie zasygnalizowany. Początkowo mnie to zmylało - ale - nasz fence tworzony jest w stanie eSignaled - więc pierwsze wywołanie waitForFences() przechodzi, bo fencje jest eSignaled.
	// dalej robimy reset fences - czyli resetujemy fence i dopiero wykonanie graphicsQueue.submit(submitInfo, *drawFences[frameIndex]) sygnalizuje fence
	// -> jeśli przeniesiemy reset fences po return - to w momencie wczesniejszego wyjścia z funkcji, fence będzie wciąż zasygnalizowany. Jeśli jednak dojedziemy do momentu
	// że resetujemy fence (a więc będzie nie zasygnalizowany), to mamy już pewność że nie wyjdziemy zanim wykonamy graphicsQueue.submit(submitInfo, *drawFences[frameIndex]) - czyli triggerujemy funkcję,
	// która będzie sygnalizować fence.

	// teraz robimy record command buffer  > i używamy image index uzyskanego wyżej
	recordCommandBuffer(imageIndex);

	// Submitting command buffer
	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	const vk::SubmitInfo submitInfo{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*getImageCompleteSemaphores[frameIndex],  // <- będzie czekać na ten semaphore  // jeśli jest więcej semaforów, to każdy demafor odpowiada kolejnemu elementowi w pWaitDstStageMask    
		.pWaitDstStageMask = &waitDestinationStageMask,   // <- to określa na gotowość którego poziomu pipeline chcemy czekać -  a chcemy czekać na gotowość do pisania kolorów
		.commandBufferCount = 1,
		.pCommandBuffers = &*commandBuffers[frameIndex],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &*renderFinishedSemaphores[imageIndex]  // <- jak będzie zrobione to będzie informować używając tego semaphore
	};


	// fence przekazany tu obejmuje tylko ten submit, ale nie obejmuje przyszłej operacji presentKHR
	graphicsQueue.submit(submitInfo, *drawFences[frameIndex]);  // <- ten drawFence informuje że renderowanie jest zakończone
	// ale nie będzie informować o tym że prezentacja została zakończona
	const vk::PresentInfoKHR presentInfoKHR
	{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*renderFinishedSemaphores[imageIndex],
		.swapchainCount = 1,
		.pSwapchains = &*swapChain,
		.pImageIndices = &imageIndex,
	};

	// Ok czyli poroblem polega na tym że graphicsQueue.presentKHR(presentInfoKHR); zostaje odpalone - czeka sobie na renderFinishedSemaphores
	// ale CPU leci dalej. Semafor renderFinishedSemaphores zostaje zresetowany w momencie jak wykona się funkcja która na niego czekała
	// np renderFinishedSemaphores -> signaled -> presentKHR zaczyna wyświetlać -> renderFinishedSemaphores -> unsignaled. Problem polega jednak na tym
	// że CPU poleciał już dale i nie dostaje informacji czy ten konkretny semafor jest wciąż używany czy już nie.
	// - mamy za to inną gwarancję -> jeśli acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex],  ...) zwróci nam obraz, to mamy gwarancję że
	// semafor przypisany do danego obrazu został już skonsumowany i możemy go użyć ( to też oznacza że operacja prezentacji została zakończona )
	result = graphicsQueue.presentKHR(presentInfoKHR);

	// presentKHR również zwraca info o sukcesie albo nie. JEśli mamy result suboptimal albo OutOfDate to odtwarzamy swap chain
	// ( nie musimy robić tu return, bo to i tak już końcówka drawFrame)
	if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || frameBufferResized)
	{
		recreateSwapChain();
	}
	else
	{
		assert(result == vk::Result::eSuccess);
	}

	frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void HelloTriangleApp::createSemaphoresAndFences()
{
	assert(getImageCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && drawFences.empty());

	// Czyli zdaje się jest tak: Potrzebujemy tyle renderFinishedSemaphores ile jest obrazów we swap chain
	// natomiast drawFences i getImageCompleteSemaphores, potrzebujemy tyle ile jest MAX_FRAMES_IN_FLIGHT
	// - po 1 nie będziemy odpalać więcej renderów niż jest MAX_FRAMES_IN_FLIGHT. Jednak w momencie gdy pobieramy
	// Image z swapChaina nie wiemy którą klatkę dostaniemy - chodzi po prostu o to, aby klatka którą dostajemy w danym
	// momencie, miała swój, przypisany do siebie renderFinishedSemaphore - niezależnie od rego ile jest Frames in flight.

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		renderFinishedSemaphores.emplace_back(logicalDevice, vk::SemaphoreCreateInfo());
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		drawFences.emplace_back(logicalDevice, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
		getImageCompleteSemaphores.emplace_back(logicalDevice, vk::SemaphoreCreateInfo());
	}
}

void HelloTriangleApp::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	// I pobieramy przekazany do window pointer na this - i zmieniamy flagę resized na true !
	auto appPtr = reinterpret_cast<HelloTriangleApp*>(glfwGetWindowUserPointer(window));
	appPtr->frameBufferResized = true;
}


