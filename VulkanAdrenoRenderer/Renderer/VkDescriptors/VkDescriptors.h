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
	void CreateDescriptorSets(std::vector<vk::raii::Buffer>* InUniformBuffers);
	
	vk::raii::DescriptorSetLayout* GetDescriptorSetLayout() { return &descriptorSetLayout; };
	std::vector<vk::raii::DescriptorSet>* GetDescriptorSets() { return &descriptorSets; };

private:
	void CreateDescriptorSetLayout();
	void CreateDescriptorPool();
	

	vk::raii::Device* LogicalDevice;
	uint32_t MaxFramesInFlight;
	
	vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;

	vk::raii::DescriptorPool descriptorPool = nullptr;
	std::vector<vk::raii::DescriptorSet> descriptorSets;
};