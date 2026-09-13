#pragma once

#include <iostream>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS  
//#define VULKAN_HPP_NO_EXCEPTIONS

#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS 

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN // czyli w glfw3.h jest #if dedined(GLFW_INCLUDE_VULCAN) -> 
#include <GLFW/glfw3.h>     

constexpr uint32_t WINDOW_WIDTH = 800;
constexpr uint32_t WINDOW_HEIGHT = 600;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

#include "GLFWWindow/WindowGLFW.h"
#include "VkContext/VkContext.h"

class VulkanRenderer
{
public:
	VulkanRenderer();

	void InitializeRenderer()
	{
		Window.InitWindow(WINDOW_WIDTH,WINDOW_HEIGHT,"VulkanAdrenoRenderer",this,framebufferResizeCallback);
		VulkanContext.InitVkContext(&Window);

		RunMainLoop();
	};

	void RunMainLoop()
	{
		MainLoop();
	}

	void MainLoop();

	//void run()
	//{
	//	//Window.InitWindow(WINDOW_WIDTH,WINDOW_HEIGHT,"VulkanRenderer", this, framebufferResizeCallback);
	//	
	//	initVulkan();
	//	mainLoop(); 
	//	cleanup();
	//}

private:

	WindowGLFW Window;
	VkContext VulkanContext;

	/// <summary>
	/// 
	/// </summary>
	vk::raii::SwapchainKHR swapChain = nullptr;
	std::vector<vk::Image> swapChainImages;
	std::vector<vk::raii::ImageView> swapChainImageViews;  // <- image views służą do dostępu do vk::Images - zawietają informację jak dany Image powinien być używany
	vk::SurfaceFormatKHR swapChainSurfaceFormat;
	vk::Extent2D swapChainExtent;

	// Pipeline
	vk::raii::PipelineLayout pipelineLayout = nullptr;
	vk::raii::Pipeline pipeline = nullptr;

	// command pool
	vk::raii::CommandPool commandPool = nullptr;

	// command buffer
	std::vector<vk::raii::CommandBuffer> commandBuffers;

	// Semaphores i fences - syncObjects
	std::vector<vk::raii::Semaphore> getImageCompleteSemaphores; // <- będzie wskażywać że Image został uzyskany ze swapchaina i jest gotowy do renderingu
	std::vector<vk::raii::Semaphore> renderFinishedSemaphores;  // <- będzie informować o tym że renderowanie zsotało zakończone
	std::vector<vk::raii::Fence>     drawFences;				// <- fence będzie służyć do zapewnienia że renderujemy tylko jedną klatkę w danym czasie 

	uint32_t frameIndex = 0;

	// GLFW
	GLFWwindow* window = nullptr;
	bool frameBufferResized = false;

	int previousWidth = 0;
	int prewiousHeight = 0;

	
	void initVulkan()
	{
		createInstance();
		// setupDebugMessenger();  <- do zaimplementowania później
		createSurface(); // <- surface tworzymy przed wyborem karty, bo tworzenie surface może wpłynąć na wybór device
		pickPhysicalDevice(); // <- wybierając kartę graficznę, można wybrać dowolną ilość kart któe spełniają wymagania i użyć ich symultanicznie
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createGraphicsPipeline();
		createCommandPool();
		createCommandBuffers();
		createSemaphoresAndFences();  // <- ( create sync objects ) 
	}

	// Recreate swap chain jest potrzebne np w sytuacji gdy zmienimy rozmiar okna i istniejący swapChain jest już nie aktualny
	// ( no bo właśnie zmieniły się rozmiar czyli powinien się zmienić również rozmiar Images ) 
	void recreateSwapChain()
	{
		// jest jeden specjalny przypadek -> jeśli minimalizujemy okno, to jego rozmiar jest 0 , 0 -> obecnie po prostu zawieszamy w pętli while program
		int width = 0;
		int height = 0;

		glfwGetFramebufferSize(window, &width, &height);

		while (width == 0 || height == 0)  // <- wystarczy że jedna z wartości będzie 0 i taki framebuffer będzie nieprawidłowy
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();  // <- glfwWaitEvents() na chwilę usypia bieżączy wątek i czeka na jakieś zdarzenie i po tym zdarzeniu aktualizuje glfw
			// Jeśli tego nie ma to po minimalizacji okna while kręci się cały czas i wątek jest zablokowany i nie możliwe jest przywrócenie okna
		}

		VulkanContext.logicalDevice.waitIdle();  // <- waitIdle() bo nie powinniśmy używać zasobów które są w użyciu 
		cleanupSwapChain();
		createSwapChain();
		createImageViews();
	}

	//void mainLoop()
	//{

	//	glfwGetWindowSize(window, &previousWidth, &prewiousHeight);
	//	while (!glfwWindowShouldClose(window))
	//	{
	//		glfwPollEvents(); 
	//		drawFrame();
	//	}
	//	logicalDevice.waitIdle();  // <- jak zamykamy okno to może crashować, bo zamykamy je ale semafory są np są otwerte bo coś wię cykonuje asynchronicznie. Czekamy więc aż karta zakończy
	//}

	void Cleanup()
	{
		Window.DestroyWindow();
	}

	// Functions Defined in cpp
	void createInstance();
	std::vector<const char*> getRequiredInstanceExtensions();

	// Surface
	void createSurface();

	// Physical Device functions - te funkcje służą wybraniu konkretnej karty i przypisaniu jej do physicalDevice
	void pickPhysicalDevice();
	bool isDeviceSuitable(vk::raii::PhysicalDevice const& InPhysicalDevice);  // <- przykładowa funkcja pozwalająca na sprawdzenie czy dana fizyczna karta jest odpowiednia
	vk::raii::PhysicalDevice* choosePhysicalDeviceByScore(std::vector<vk::raii::PhysicalDevice>& physicalDevices);

	// Logical Device functions - tu tworzymy logical Device - czyli główny obiekt komunikacji pomiędzy aplikacją a GPU
	void createLogicalDevice();

	// Swap Chain
	void createSwapChain();
	vk::SurfaceFormatKHR chooseSwapChainSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats);
	vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);
	vk::Extent2D chooseSwapChainExtent(vk::SurfaceCapabilitiesKHR const& capabilities);
	uint32_t chooseSwapChainMinImageCount(vk::SurfaceCapabilitiesKHR const& capabilities);
	void cleanupSwapChain();

	// ImageViews
	void createImageViews();

	// GraphicsPipeline
	void createGraphicsPipeline();
	static std::vector<uint32_t> readFile(const std::string& filename);

	[[nodiscard]]
	vk::raii::ShaderModule createShaderModule(const std::vector<uint32_t>& shaderBytes);


	// Command pool
	void createCommandPool();

	// Command Buffer
	void createCommandBuffers();

	// Record CommandBuffer
	void recordCommandBuffer(uint32_t imageIndex);

	// transition image layout zostanie użyte do przerobienia image z ImageLayout::eUndefined na ::eColorAttachementOptimal
	void transition_image_layout(          // w zależności od tego do czego ma być użyty Image, należy zmienić jego layout
		uint32_t imageIndex,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		vk::AccessFlags2 src_access_mask,
		vk::AccessFlags2 dst_access_mask,
		vk::PipelineStageFlags2 src_stage_mask,
		vk::PipelineStageFlags2 dst_stage_mask
	);

	// Draw frame ( in main loop ) 
	void drawFrame();

	// Sync objects
	void createSemaphoresAndFences();

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

};