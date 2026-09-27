#pragma once

#include "../VulkanCommon.h"
#include "VertexFactory.h"

enum class EBufferDataUploadMode
{
	Direct, // <- bezpośredni upload danych do pamięci GPU ( może być wolniejszy, ale może być szybszy dla kart z współdzielonym z CPU ramem )
	Staging  // <- staging buffer - czyli najpierw kopiujemy dane do bufora w pamięci CPU a następnie do osobnego bufora w pamięci GPU ( może być szybsze dla kart z własnym VRAM )
};

// jeśli GPU ma własny VRAM to vk::MemoryPropertyFlagBits::eDeviceLocal (czyli jego własny ram) jest najlepszą
// pamięcią do odczytu dla niego. ( jednocześnie, zwykle ta pamięć nie jest dostępna dla CPU )

// Normalny buffer VS tworzenie staging buffer
// normalny/Direct : 1) W klasie Vk_VertexBuffer mamy pole vertexBuffer i vertexBufferMemory. 
//			         2) Tworzymy obiekty bufforów z typem pamięci która jest mapowalna dla CPU. 
//					 3) Po utworzeniu pamięci kopiujemy dane bezpośrednio do tej pamięci i później GPU jej używa

// Staging Buffer:
//					 1) W funkcji najpierw tworzymy staging buffer - czyli bufor pośredni który jest mapowalny dla CPU ( te bufory nie są polem klasy, istnieją tylko na czas funkcji )
//					 2) Do staging bufferów kopiujemy dane ( vertexy w tym przypadku )
//					 3) Tworzymy obiekty buforów które są polem klasy - vertexBuffer i vertexBufferMemory - w tym przypadku nie jest to pamięć mapowalna dla CPU a eDeviceLocal
//					 4) Używamy specjalnej funkcji kopiującej dane między buforami i ze staging buffer kopijemy dane do właściwego bufora eDeviceLocal. Jeśli GPU ma własny vram to jest szybciej, bo używamy sprcjalnej funkcji 
//						kopiującej dane, a GPU czyta dane bezpośrednio z VRAM

class Vk_VertexBuffer
{
public:
	Vk_VertexBuffer() {};

	void InitVkVertexBuffer(vk::raii::Device* InLogicalDevice,vk::raii::PhysicalDevice* InPhysicalDevice,EBufferDataUploadMode InUploadMode)
	{
		uint32_t VertexBufferSize = sizeof(Vertex) * vertices.size();
		CreateVertexBuffer(InLogicalDevice,InPhysicalDevice,VertexBufferSize,InUploadMode);
	};

	vk::raii::Buffer* GetVertexBuffer() { return &vertexBuffer; };

	void CopyVerticesToBuffer(uint32_t InMemoryToMapSize);

private:

	void CreateVertexBuffer(vk::raii::Device* InDevice, vk::raii::PhysicalDevice* InPhysicalDevice, uint32_t InBufferSize, EBufferDataUploadMode InUploadMode);
	
	void CreateUsingDirectBuffer(vk::raii::Device* InDevice,
		vk::raii::PhysicalDevice* InPhysicalDevice,
		uint32_t InBufferSize,
		vk::BufferUsageFlagBits InBufferUsage,
		vk::MemoryPropertyFlags InMemoryProperties);
	void CreateUsingStagingBuffer(vk::raii::Device* InDevice,
		vk::raii::PhysicalDevice* InPhysicalDevice,
		uint32_t InBufferSize,
		vk::BufferUsageFlagBits InStageBufferUsage, 
		vk::MemoryPropertyFlags InStageBufferMemoryProperties, 
		vk::BufferUsageFlagBits InDestBufferUsage, 
		vk::MemoryPropertyFlags InDestBufferMemoryProperties
	);
	
	uint32_t FindMemoryTypeIndex(vk::raii::PhysicalDevice* InPhysicalDevice, uint32_t InTypeFilter, vk::MemoryPropertyFlags InProperties);  // <- generalnie GPU oferują różne typy pamięci, o różnej wydajności i zastosowaniach - musimy znaleźć właściwy
	
	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> CreateBuffer(
		vk::raii::Device* InDevice,
		vk::raii::PhysicalDevice* InPhysicalDevice,
		uint32_t InBufferSize,
		vk::BufferUsageFlagBits InBufferUsage,
		vk::MemoryPropertyFlags InMemoryProperties
	);
	
	vk::raii::Buffer vertexBuffer = nullptr;
	vk::raii::DeviceMemory vertexBufferMemory = nullptr;
};