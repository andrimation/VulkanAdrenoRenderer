#pragma once

#include "../VulkanCommon.h"
#include "VertexFactory.h"

enum class EBufferType
{
	Vertex, // <- bezpośredni upload danych do pamięci GPU ( może być wolniejszy, ale może być szybszy dla kart z współdzielonym z CPU ramem )
	Index  // <- staging buffer - czyli najpierw kopiujemy dane do bufora w pamięci CPU a następnie do osobnego bufora w pamięci GPU ( może być szybsze dla kart z własnym VRAM )
};

enum class EBufferDataUploadMode
{
	Direct, // <- bezpośredni upload danych do pamięci GPU ( może być wolniejszy, ale może być szybszy dla kart z współdzielonym z CPU ramem )
	Staging  // <- staging buffer - czyli najpierw kopiujemy dane do bufora w pamięci CPU a następnie do osobnego bufora w pamięci GPU ( może być szybsze dla kart z własnym VRAM )
};


// jeśli GPU ma własny VRAM to vk::MemoryPropertyFlagBits::eDeviceLocal (czyli jego własny ram) jest najlepszą
// pamięcią do odczytu dla niego. ( jednocześnie, zwykle ta pamięć nie jest dostępna dla CPU )

// Normalny buffer VS tworzenie staging buffer
// normalny/Direct : 1) W klasie Vk_Buffers mamy pole vertexBuffer i vertexBufferMemory. 
//			         2) Tworzymy obiekty bufforów z typem pamięci która jest mapowalna dla CPU. 
//					 3) Po utworzeniu pamięci kopiujemy dane bezpośrednio do tej pamięci i później GPU jej używa
// 
// Staging Buffer:
//					 1) W funkcji najpierw tworzymy staging buffer - czyli bufor pośredni który jest mapowalny dla CPU ( te bufory nie są polem klasy, istnieją tylko na czas funkcji )
//					 2) Do staging bufferów kopiujemy dane ( vertexy w tym przypadku )
//					 3) Tworzymy obiekty buforów które są polem klasy - vertexBuffer i vertexBufferMemory - w tym przypadku nie jest to pamięć mapowalna dla CPU a eDeviceLocal
//					 4) Używamy specjalnej funkcji kopiującej dane między buforami i ze staging buffer kopiujemy dane do właściwego bufora eDeviceLocal. Jeśli GPU ma własny vram to jest szybciej, bo używamy sprcjalnej funkcji 
//						kopiującej dane, a GPU czyta dane bezpośrednio z VRAM

class Vk_Buffers
{
public:
	Vk_Buffers() {};

	void InitVkBuffers(
		vk::raii::Device* InLogicalDevice,
		vk::raii::PhysicalDevice* InPhysicalDevice,
		uint32_t InMaxFramesInFlight,
		EBufferDataUploadMode InUploadMode = EBufferDataUploadMode::Direct, 	
		vk::raii::CommandPool* InCommandPool = nullptr, 
		vk::raii::Queue* InGraphicsQueue = nullptr
	)
	{
		LogicalDevice = InLogicalDevice;
		PhysicalDevice = InPhysicalDevice;
		MaxFramesInFlight = InMaxFramesInFlight;
		UploadMode = InUploadMode;
		CommandPool = InCommandPool;
		GraphicsQueue = InGraphicsQueue;

		CreateBuffers();
		CreateUniformBuffers();
	};

	vk::raii::Buffer* GetVertexBuffer() { return &vertexBuffer; };
	vk::raii::Buffer* GetIndexBuffer()  { return &indexBuffer;  };
	std::vector<vk::raii::Buffer>* GetUniformBuffers() { return &uniformBuffers; };
	void* GetUniformBufferMapped(uint32_t InIndex)  { return uniformBuffersMapped[InIndex];  };

	// UWAGA -> Generalnie istnieje też sposób łączenia bufferów np vertex i index w jeden bufor, żeby było
	// bardziej cache friendly.

private:

	void CreateBuffers();		
	void CreateUsingDirectBuffer(vk::BufferUsageFlagBits InBufferUsage);
	void CreateUsingStagingBuffer(vk::BufferUsageFlagBits InBufferUsage);
		
	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> CreateBuffer(uint32_t InBufferSize, vk::BufferUsageFlags InBufferUsage, vk::MemoryPropertyFlags InMemoryProperties);

	void CreateUniformBuffers();

	uint32_t GetBufferMemorySize(vk::BufferUsageFlagBits InBufferUsage);
	const void* FindDataSourceToCopy(vk::BufferUsageFlagBits InBufferUsage);
	void CopyVerticesToBuffer(vk::raii::DeviceMemory& InDestBufferMemory,const void* InDataSource, uint32_t InMemoryToMapSize);
	void CopyBuffer(vk::raii::Buffer* InSrcBuffer, vk::raii::Buffer* InDstBuffer, uint32_t Size);

	uint32_t FindMemoryTypeIndex(uint32_t InTypeFilter, vk::MemoryPropertyFlags InProperties);  // <- generalnie GPU oferują różne typy pamięci, o różnej wydajności i zastosowaniach - musimy znaleźć właściwy
	
	// Buffers and Buffers Memory
	struct BufferStorage  // helper struct to assign proper buffer and buffer memory, depending on creating buffer type
	{
		vk::raii::Buffer& Buffer;
		vk::raii::DeviceMemory& BufferMemory;
	};

	Vk_Buffers::BufferStorage GetBufferStorage(vk::BufferUsageFlagBits InBufferUsage);

	vk::raii::Device* LogicalDevice;
	vk::raii::PhysicalDevice* PhysicalDevice;
	uint32_t MaxFramesInFlight;
	EBufferDataUploadMode UploadMode;
	vk::raii::CommandPool* CommandPool;
	vk::raii::Queue* GraphicsQueue;

	vk::raii::Buffer vertexBuffer = nullptr;
	vk::raii::DeviceMemory vertexBufferMemory = nullptr;
	vk::raii::Buffer indexBuffer = nullptr;
	vk::raii::DeviceMemory indexBufferMemory = nullptr;

	std::vector<vk::raii::Buffer> uniformBuffers;
	std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;
};