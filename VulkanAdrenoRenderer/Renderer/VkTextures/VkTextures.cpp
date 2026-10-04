#include "VkTextures.h"
#include "../VertexBuffer/VertexBuffer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void Vk_Textures::CreateTextureImage()
{
	int textureWidth, textureHeight, textureChannels;
	stbi_uc* pixels = stbi_load(
		"textures/texture.jpg",
		&textureWidth,
		&textureHeight,
		&textureChannels,
		STBI_rgb_alpha   // Force the image to load with 4 channels (RGBA)
	);

	if (!pixels)
	{
		throw std::runtime_error("failed to load texture image!");
	}

	vk::DeviceSize imageSize = textureWidth * textureHeight * 4;  // 4 bytes per pixel (RGBA)

	

	// Load to staging buffer
	auto [stagingBuffer, stagingBufferMemory] = VulkanBuffersObject->CreateBuffer(
		imageSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | 
		vk::MemoryPropertyFlagBits::eHostCoherent  // <- onznacza ze dane są zapisywane natychmiast a nie cachowane gdzieś
	);

	void* bufferMapped = stagingBufferMemory.mapMemory(0, imageSize);
	memcpy(bufferMapped, pixels, static_cast<size_t>(imageSize));
	stagingBufferMemory.unmapMemory();

	stbi_image_free(pixels);
}
