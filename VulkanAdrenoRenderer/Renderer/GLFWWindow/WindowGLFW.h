#pragma once

#define GLFW_INCLUDE_VULKAN // czyli w glfw3.h jest #if dedined(GLFW_INCLUDE_VULCAN) -> 
#include <GLFW/glfw3.h>    

class VulkanRenderer;

class WindowGLFW
{
public:
	WindowGLFW() {};

	void InitWindow(int window_width, int window_height, const char* window_title, void* InRendererPtr, GLFWframebuffersizefun InResizeCallback);
	void DestroyWindow();

	GLFWwindow* window;
};