#pragma once

#include "../VulkanCommon.h"

class Vk_Buffers;

class Vk_Textures
{
public:

	Vk_Textures() = default;

	void InitVkTextures(Vk_Buffers* InVkBuffers)
	{
		VulkanBuffersObject = InVkBuffers;

		CreateTextureImage();
	};

	void CreateTextureImage();

private:

	Vk_Buffers* VulkanBuffersObject;

	vk::raii::Image textureImage = nullptr;
	vk::raii::DeviceMemory textureImageMemory = nullptr;
};
