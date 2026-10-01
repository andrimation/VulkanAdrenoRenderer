#include "VkDescriptors.h"

#include "../VkContext/VkContext.h"

void Vk_Descriptors::InitVk_Descriptors(vk::raii::Device* InLogicalDevice, uint32_t InMaxFramesInFlight)
{
	MaxFramesInFlight = InMaxFramesInFlight;
	LogicalDevice = InLogicalDevice;
	CreateDescriptorSetLayout();
}

void Vk_Descriptors::CreateDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding uboLayoutBinding{
		.binding = 0,  // <- ten binding jest dostępny pod indexem 0
		.descriptorType = vk::DescriptorType::eUniformBuffer, // pod indexem 0 będzie się znajdował uniform buffer
		.descriptorCount = 1, // pod indexem 0 będzie 1 descriptor tego typu
		.stageFlags = vk::ShaderStageFlagBits::eVertex // <- informuje na którym etapie shadera będzie dostępny ten descriptor
	};

	// DescriptorSetLatout - schemat danych
	// DescriptorSet       - konkretne dane zgodne ze schematem
	vk::DescriptorSetLayoutCreateInfo layoutInfo
	{
		.bindingCount = 1,
		.pBindings = &uboLayoutBinding
	};

	// Przypisujemy opis descriptora do descriptorSetLayout, a następnie przekażemy descriptorSetLayout 
	// do PipelineLayoutCreateInfo
	descriptorSetLayout = vk::raii::DescriptorSetLayout(*LogicalDevice, layoutInfo);
}

void Vk_Descriptors::CreateDescriptorPool()
{
	vk::DescriptorPoolSize poolSize = {
		.type = vk::DescriptorType::eUniformBuffer,
		.descriptorCount = MaxFramesInFlight
	};

	vk::DescriptorPoolCreateInfo poolInfo = {
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, // Pozwala zwalniać pojedyncze descriptor sety ( bez tej flagi zwalniany jest na raz cały pool )
		.maxSets = MaxFramesInFlight,
		.poolSizeCount = 1, // -> że w momencie tworzenia descriptora przeazujemy jeden obiekt vk::DescriptorPoolSize poolSize ( bo najwyraźniej można kilka )
		.pPoolSizes = &poolSize
	};

	descriptorPool = vk::raii::DescriptorPool(*LogicalDevice, poolInfo);
}

void Vk_Descriptors::CreateDescriptorSets()
{
	std::vector<vk::DescriptorSetLayout> layouts(MaxFramesInFlight, *descriptorSetLayout);

}
