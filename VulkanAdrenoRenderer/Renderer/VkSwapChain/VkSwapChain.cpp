#include "VkSwapChain.h"
#include "../VkContext/VkContext.h"
#include "../GLFWWindow/WindowGLFW.h"
#include <iostream>

void Vk_SwapChain::CreateSwapChain(Vk_Context* InContext, WindowGLFW* InWindow)
{
	// to nam zwraca dostępne dla surface formaty ( pixel format, color space )
	std::vector<vk::SurfaceFormatKHR> availableFormats = InContext->physicalDevice.getSurfaceFormatsKHR(*InContext->surface);

	// dostępne present modes ( presentation mode - czyli warunki/sposoby "swapowania" obrazów na ekranie )
	// PresentationMode jest prawdopodobnie najważniejszym settingsem dla SwapChain
	std::vector<vk::PresentModeKHR> availablePresentModes = InContext->physicalDevice.getSurfacePresentModesKHR(*InContext->surface);

	// Niżej będą funkcje, które sprawdzają czy wybrane przez nas ustawienia są dostępne, a jeśli nie to szuka innych najlepszych dostępnych
	swapChainSurfaceFormat = ChooseSwapChainSurfaceFormat(availableFormats);
	vk::PresentModeKHR   presentMode = ChooseSwapPresentMode(availablePresentModes);

	// surface capabilities zwraca nam podstawowe możliwości powierzchni (max ilość obrazów w swap chain i ich max wymiary)
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = InContext->physicalDevice.getSurfaceCapabilitiesKHR(*InContext->surface);
	swapChainExtent = ChooseSwapChainExtent(surfaceCapabilities,InWindow);

	// generalnie jest określone minimum swap chain images dla danej implementacji
	surfaceCapabilities.minImageCount;  // warto jednak mieć więcej images we swap chain niż minimum - jednocześnie musimy mieć na uwadze aby nie przekroczyć maksymalnej liczby obrazów we swap chainie
	uint32_t swapChainImageCount = ChooseSwapChainMinImageCount(surfaceCapabilities);

	std::cout << "SwapChainImageCount =  " << swapChainImageCount << "\n";

	vk::SwapchainCreateInfoKHR swapChainCreateInfo{
		.surface = *InContext->surface,
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

	swapChain = vk::raii::SwapchainKHR(InContext->logicalDevice, swapChainCreateInfo);
	swapChainImages = swapChain.getImages();
}

vk::SurfaceFormatKHR Vk_SwapChain::ChooseSwapChainSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats)
{
	const auto formatIT = std::ranges::find_if(availableFormats, [](const vk::SurfaceFormatKHR& availableFormat) {
		return (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear);
	});
	// Jeżeli iterator == availableFormats.end() to znaczy że dotarł do końca - i nic nie znalazł. 
	// jeżeli znalazł to zatrzymał się na tym elemencie. jak zrobimy *formatIT to pobierzemy element na którym zatrzymał się iterator
	return formatIT != availableFormats.end() ? *formatIT : availableFormats[0];
}

vk::PresentModeKHR Vk_SwapChain::ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes)
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

vk::Extent2D Vk_SwapChain::ChooseSwapChainExtent(vk::SurfaceCapabilitiesKHR const& capabilities, WindowGLFW* InWindow)
{
	// To jest normalny przypadek - jako Extent2D zwracamy rozmiar okna
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}

	// Niektóre menedżery okien pozwalają na zmianę rozdzielczości, co sygnalizują ustawiając rozmiar SurfaceCapabilitiesKHR na std::numeric_limits<uint32_t>::max()
	// w takim przypadku pobieramy faktyczny rozmiaz z frame buffera glfw -
	int width, height;
	glfwGetFramebufferSize(InWindow->window, &width, &height);

	// jak pobierzemy fasktyczne rozmiary z frame buffera, to clampujemy je - jeśli np rozmiar jest mniejszy niż minImageExtent - to zwracamy minImageExtend
	// jeśli większy niż maxImageExtent - to analogicznie zwracamy maxImageExtent
	return {
		std::clamp<uint32_t>(width,capabilities.minImageExtent.width,capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(width,capabilities.minImageExtent.height,capabilities.maxImageExtent.height)
	};
}

uint32_t Vk_SwapChain::ChooseSwapChainMinImageCount(vk::SurfaceCapabilitiesKHR const& capabilities)
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

void Vk_SwapChain::CleanupSwapChain()
{
	swapChainImageViews.clear();
	swapChain = nullptr;
}

void Vk_SwapChain::CreateImageViews(Vk_Context* InContext)
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
		swapChainImageViews.emplace_back(InContext->logicalDevice, imageViewCreateInfo);
	}
}