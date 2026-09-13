// VulkanAdrenoRenderer.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include <iostream>

#include "VulkanRenderer.h"

class VulkanApplication
{
public:

	VulkanApplication();
	void Init();

private:
	VulkanRenderer Renderer;
};
