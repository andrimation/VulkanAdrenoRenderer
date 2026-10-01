#pragma once

#include "../VulkanCommon.h"

class Vk_Context;

// DescriptorSetLatout pozwala na przekazywanie do shadera globalnych danych np matryc transformacji, czy innych danych które nie są vertexami
// Generalnie do przekazania DescriprorSet używa się bufferów 

class Vk_Descriptors
{
public:
	Vk_Descriptors() = default;

	void InitVk_Descriptors(vk::raii::Device* InLogicalDevice, uint32_t InMaxFramesInFlight);	
	
	vk::raii::DescriptorSetLayout* GetDescriptorsSetLayout() { return &descriptorSetLayout; };

private:
	void CreateDescriptorSetLayout();
	void CreateDescriptorPool();
	void CreateDescriptorSets();

	vk::raii::Device* LogicalDevice;
	uint32_t MaxFramesInFlight;
	
	vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;

	vk::raii::DescriptorPool descriptorPool = nullptr;
	std::vector<vk::raii::DescriptorSet> descriptorSets;
};