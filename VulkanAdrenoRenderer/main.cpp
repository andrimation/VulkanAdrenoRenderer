#pragma once
#include <iostream>

#include "Application/VulkanApplication.h"

int main()
{
	std::cout << "Create Renderer" << std::endl;

	VulkanApplication VkApp;
	VkApp.Init();

	return 0;
}