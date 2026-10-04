#include "VkDescriptors.h"

#include "../VkContext/VkContext.h"
#include "../VkSceneTransforms/VkSceneTransforms.h"

void Vk_Descriptors::InitVk_Descriptors(vk::raii::Device* InLogicalDevice, uint32_t InMaxFramesInFlight)
{
	MaxFramesInFlight = InMaxFramesInFlight;
	LogicalDevice = InLogicalDevice;
	
	CreateDescriptorSetLayout();

	CreateDescriptorPool();
	//CreateDescriptorSets();
}

void Vk_Descriptors::CreateDescriptorSetLayout()
{
	// Tu tworzymy jakby opis descriptorSets

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
	// Tworzymy pool dla descriptorSets - w pool tyle ile MaxFramesInFlight - czyli de facto jeden descriptorSets per frame in flight

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

void Vk_Descriptors::CreateDescriptorSets(std::vector<vk::raii::Buffer>* InUniformBuffers)
{
	std::vector<vk::DescriptorSetLayout> layouts(MaxFramesInFlight, *descriptorSetLayout);

	vk::DescriptorSetAllocateInfo allocInfo{
		.descriptorPool = descriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
		.pSetLayouts = layouts.data()
	};

	descriptorSets = LogicalDevice->allocateDescriptorSets(allocInfo);

	for (int i = 0; i < MaxFramesInFlight; i++)
	{
		vk::DescriptorBufferInfo bufferInfo
		{
			.buffer = (*InUniformBuffers)[i],
			.offset = 0,
			.range = sizeof(SceneTransformMatrices)
		};

		vk::WriteDescriptorSet descriptorWrite
		{
			.dstSet = descriptorSets[i],
			.dstBinding = 0,  // <- ten binding odpowiada bindingowi 0 z CreateDescriptorSetLayout()
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.pBufferInfo = &bufferInfo
		};

		LogicalDevice->updateDescriptorSets(descriptorWrite, {});
	}
}
