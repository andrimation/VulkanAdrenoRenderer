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
#include "VkSwapChain/VkSwapChain.h"
#include "VkPipeline/VkPipeline.h"
#include "VkCommands/VkCommands.h"
#include "VkSynchronization/VkSynchronization.h"
#include "VertexBuffer/VertexFactory.h"
#include "VertexBuffer/VertexBuffer.h"

class VulkanRenderer
{
public:
	VulkanRenderer();

	void InitializeRenderer()
	{
		Window.InitWindow(WINDOW_WIDTH,WINDOW_HEIGHT,"VulkanAdrenoRenderer",this,framebufferResizeCallback);
		VulkanContext.InitVkContext(&Window);
		VulkanSwapChain.InitVkSwapChain(&VulkanContext,&Window);
		VulkanPipeline.InitVkPipeline(&VulkanContext, &Window, &VulkanSwapChain);
		VulkanVertexBuffer.InitVkVertexBuffer(&VulkanContext.logicalDevice,&VulkanContext.physicalDevice); // <- bufory powinny być utworzone przed RecordCommands - żeby mogły być dostępne w momencie nagrywania command
		VulkanCommands.InitVkCommands(&VulkanContext, &VulkanSwapChain);
		VulkanSynchronization.InitVkSynchronization(&VulkanContext, &VulkanSwapChain, MAX_FRAMES_IN_FLIGHT);

		RunMainLoop();
	};

	void RunMainLoop()
	{
		MainLoop();
	}

	void MainLoop();

private:

	WindowGLFW Window;
	Vk_Context VulkanContext;
	Vk_SwapChain VulkanSwapChain;
	Vk_Pipeline VulkanPipeline;
	Vk_Commands VulkanCommands;
	Vk_Synchronization VulkanSynchronization;
	Vk_VertexBuffer VulkanVertexBuffer;
	
	bool frameBufferResized = false;

	int previousWidth = 0;
	int prewiousHeight = 0;

	
	// Recreate swap chain jest potrzebne np w sytuacji gdy zmienimy rozmiar okna i istniejący swapChain jest już nie aktualny
	// ( no bo właśnie zmieniły się rozmiar czyli powinien się zmienić również rozmiar Images ) 
	void recreateSwapChain()
	{
		// jest jeden specjalny przypadek -> jeśli minimalizujemy okno, to jego rozmiar jest 0 , 0 -> obecnie po prostu zawieszamy w pętli while program
		int width = 0;
		int height = 0;

		glfwGetFramebufferSize(Window.window, &width, &height);

		while (width == 0 || height == 0)  // <- wystarczy że jedna z wartości będzie 0 i taki framebuffer będzie nieprawidłowy
		{
			glfwGetFramebufferSize(Window.window, &width, &height);
			glfwWaitEvents();  // <- glfwWaitEvents() na chwilę usypia bieżączy wątek i czeka na jakieś zdarzenie i po tym zdarzeniu aktualizuje glfw
			// Jeśli tego nie ma to po minimalizacji okna while kręci się cały czas i wątek jest zablokowany i nie możliwe jest przywrócenie okna
		}

		VulkanContext.logicalDevice.waitIdle();  // <- waitIdle() bo nie powinniśmy używać zasobów które są w użyciu 
		VulkanSwapChain.RecreateVkSwapChain(&VulkanContext,&Window);
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

	void DrawFrame(Vk_Context* InContext, Vk_SwapChain* InSwapChain, Vk_Pipeline* InPipeline);

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

};