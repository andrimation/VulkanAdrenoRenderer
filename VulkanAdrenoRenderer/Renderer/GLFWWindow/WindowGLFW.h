#pragma once

#include "../VulkanCommon.h" 

class VulkanRenderer;

class WindowGLFW
{
public:
	WindowGLFW() {};

	void InitWindow(int window_width, int window_height, const char* window_title, void* InRendererPtr, GLFWframebuffersizefun InResizeCallback);
	void DestroyWindow();
	void SetTitle(const std::string& InTitle);

	GLFWwindow* window;
};