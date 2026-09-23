#pragma once

#include "../VulkanCommon.h"
#include "VertexFactory.h"


class Vk_VertexBuffer
{
public:
	Vk_VertexBuffer() {};

	void InitVkVertexBuffer(vk::raii::Device* InLogicalDevice,vk::raii::PhysicalDevice* InPhysicalDevice)
	{
		uint32_t VertexBufferSize = sizeof(Vertex) * vertices.size();
		CreateVertexBuffer(InLogicalDevice,InPhysicalDevice,VertexBufferSize);
	};

	vk::raii::Buffer* GetVertexBuffer() { return &vertexBuffer; };

	void CopyVerticesToBuffer(uint32_t InMemoryToMapSize);

private:

	void CreateVertexBuffer(vk::raii::Device* InDevice, vk::raii::PhysicalDevice* InPhysicalDevice, uint32_t InBufferSize);
	uint32_t FindMemoryTypeIndex(vk::raii::PhysicalDevice* InPhysicalDevice, uint32_t InTypeFilter, vk::MemoryPropertyFlags InProperties);  // <- generalnie GPU oferują różne typy pamięci, o różnej wydajności i zastosowaniach - musimy znaleźć właściwy
	
	vk::raii::Buffer vertexBuffer = nullptr;
	vk::raii::DeviceMemory vertexBufferMemory = nullptr;
};