#include "VertexBuffer.h"
#include "VertexBuffer.h"
#include <iostream>

#include "../VkSceneTransforms/VkSceneTransforms.h"


void Vk_Buffers::CreateBuffers()
{	
	// Host To CPU
	if (UploadMode == EBufferDataUploadMode::Direct)
	{
		
		CreateUsingDirectBuffer(vk::BufferUsageFlagBits::eVertexBuffer);
		CreateUsingDirectBuffer(vk::BufferUsageFlagBits::eIndexBuffer);

	}
	else if (UploadMode == EBufferDataUploadMode::Staging)
	{

		CreateUsingStagingBuffer(vk::BufferUsageFlagBits::eVertexBuffer);
		CreateUsingStagingBuffer(vk::BufferUsageFlagBits::eIndexBuffer);

	}
}

void Vk_Buffers::CreateUsingDirectBuffer(vk::BufferUsageFlagBits InBufferUsage)
{
	// persistently mapped vertex buffer <- rozkminić co to jest i jego synchronizacja z CPU

	vk::BufferUsageFlagBits DirectBufferUsageFlags = InBufferUsage;  // <- czyli że będzie vertex albo index buffer
	vk::MemoryPropertyFlags DirectBufferMemoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent; // <- pamięć dostępna dla CPU i GPU (?)

	uint32_t BufferMemorySize = GetBufferMemorySize(InBufferUsage);
	auto [buffer, bufferMemory] = CreateBuffer(BufferMemorySize, DirectBufferUsageFlags, DirectBufferMemoryProperties);

	BufferStorage PackedBuffer = GetBufferStorage(InBufferUsage);
	PackedBuffer.Buffer = std::move(buffer);
	PackedBuffer.BufferMemory = std::move(bufferMemory);

	// Kopiujemy vertices bezpośrednio do wspólnego bufora
	const void* DataToCopy = FindDataSourceToCopy(InBufferUsage);
	CopyVerticesToBuffer(PackedBuffer.BufferMemory, DataToCopy, BufferMemorySize);
}

void Vk_Buffers::CreateUsingStagingBuffer(vk::BufferUsageFlagBits InBufferUsage)
{
	// Staging Buffer
	vk::BufferUsageFlagBits StageBufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;  // <- czyli że będzie źródłem transferu do GPU
	vk::MemoryPropertyFlags StageBufferMemoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent; // <- pamięć dostępna dla CPU i GPU (?)
	
	uint32_t BufferMemorySize = GetBufferMemorySize(InBufferUsage);

	auto [stagingBuffer, stagingBufferMemory] = CreateBuffer(BufferMemorySize, StageBufferUsageFlags, StageBufferMemoryProperties);

	// kopiujemy dane do staging buffera
	const void* DataToCopy = FindDataSourceToCopy(InBufferUsage);
	CopyVerticesToBuffer(stagingBufferMemory, DataToCopy, BufferMemorySize);


	// Destination buffer
	vk::BufferUsageFlags DestBufferUsageFlags = InBufferUsage | vk::BufferUsageFlagBits::eTransferDst;  // <- czyli że będzie buforem docelowym, do którego będziemy przenosić dane
	vk::MemoryPropertyFlags DestBufferMemoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal; // <- czyli że pamięć dostępna tylko dla CPU

	// std::tie "rozpakowuje" tuple zwracaną przez CreateBuffer() i przypisuje wyniki bezpośrednio do vertexBuffer,vertexBufferMemory ( czyli nie robimy osbnego move temp itp )
	BufferStorage PackedBuffer = GetBufferStorage(InBufferUsage);
	std::tie(PackedBuffer.Buffer, PackedBuffer.BufferMemory) = CreateBuffer(BufferMemorySize, DestBufferUsageFlags, DestBufferMemoryProperties);

	// kopiujemy staging buffer 
	CopyBuffer(&stagingBuffer,&PackedBuffer.Buffer, BufferMemorySize);
}

// Ta funkcja już nie używa pola buffer w klasie, tylko alokuje nowy i zwraca parę <Buffer, DeviceMemory> - w tym przypadku nie musimy już używać pola buffer w klasie, bo możemy zwrócić parę i przypisać ją do pola w klasie.
std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> Vk_Buffers::CreateBuffer(uint32_t InBufferSize, vk::BufferUsageFlags InBufferUsage, vk::MemoryPropertyFlags InMemoryProperties)
{
	vk::BufferCreateInfo bufferInfo{
		.size = InBufferSize,
		.usage = InBufferUsage, // <- używając bitwise or możemy zrobić że bufor nie będzie wyłącznie jako eVertexBuffer, ale że może mieć kilka zastosowań na raz. 
		.sharingMode = vk::SharingMode::eExclusive       // <- czy bufor będzie używany wyłącznie przez jedną queue czy zakładamy że może być używany przez różne
	};

	vk::raii::Buffer buffer = vk::raii::Buffer(*LogicalDevice, bufferInfo);  // <- w tym momencie mamy utworzony obiekt bufora, ale nie została jeszcze zaalokowana dla niego pamięć)
	vk::MemoryRequirements memoryRequirements = buffer.getMemoryRequirements(); // <- pobieramy wymagania jakie musi spełnić pamięć do przydzielenia dla tego bufora

	vk::MemoryAllocateInfo memoryAllocateInfo{
		.allocationSize = memoryRequirements.size,
		.memoryTypeIndex = FindMemoryTypeIndex(
			memoryRequirements.memoryTypeBits,
			InMemoryProperties	// <- właściwości które pamięć musi spełniać. 	
		)
	};

	vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(*LogicalDevice, memoryAllocateInfo);
	buffer.bindMemory(*bufferMemory, 0);

	return { std::move(buffer),std::move(bufferMemory) };
}

void Vk_Buffers::CreateUniformBuffers()
{
	for (size_t i = 0; i < MaxFramesInFlight; i++)
	{
		vk::DeviceSize bufferSize = sizeof(SceneTransformMatrices);

		auto [buffer, bufferMemory] = CreateBuffer(
			bufferSize,
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);

		uniformBuffers.push_back(std::move(buffer));
		uniformBuffersMemory.push_back(std::move(bufferMemory));

		// Mapowanie buforów kosztuje, więc utrzymujemy bufory zmapowane przez cały okres działania programu
		// i dzięki temu mamy do nich dostęp z tego vectora - i możemy przekazywać na bieżąco do niego dane
		uniformBuffersMapped.push_back(uniformBuffersMemory.back().mapMemory(0, bufferSize));
	}
}

void Vk_Buffers::CopyVerticesToBuffer(vk::raii::DeviceMemory& InDestBufferMemory,const void* InDataSource,uint32_t InMemoryToMapSize)
{
	// mapujemy najpierw pamięć bufora na pamięć dostępną dla CPU
	void* mappedMemory = InDestBufferMemory.mapMemory(0, InMemoryToMapSize);
	memcpy(mappedMemory, InDataSource, InMemoryToMapSize);  // zapamiętać że memcpy kopiować z vector.data()
	InDestBufferMemory.unmapMemory();
}

// Funkcja poniżej służy aby jeden bufor przekopiować do innego
void Vk_Buffers::CopyBuffer(vk::raii::Buffer* InSrcBuffer, vk::raii::Buffer* InDstBuffer, uint32_t InSize)
{
	// Data transfer z eTransferSrc do eTransferDst odbywa się za pomocą nagrania command buffera - więc kopiowanie danych ze Staging Bufforu do dest bufforu jest operacją wykonywaną przez GPU - musimy nagrać command, który gpu wykona
	// szczęśliwie nasza główna queue obsługuje commandy związane z transferem. Nagrywamy więc command buffer z poleceniem transferu danych i później robimy submit tego commanda do naszej queue
	vk::CommandBufferAllocateInfo CmdBuffAlocationInfo
	{
		.commandPool = *CommandPool, // podać command pool i przekazać queue
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};
	
	// zapis poniżej zwraca nam dostęp do jednego command buffera
	vk::raii::CommandBuffer CommandCopyBuffer = std::move(LogicalDevice->allocateCommandBuffers(CmdBuffAlocationInfo).front());
	// w przeciwieństwie do zapisu: commandBuffers = vk::raii::CommandBuffers(InContext->logicalDevice, commandBufferAllocateInfo);
	// który zwraca dostęp do całej kolekcji utworzonych cmd bufferów

	// Zaczynamy record commands
	CommandCopyBuffer.begin({
		.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit // <- informujemy że command buffer będzie wykonany tlko raz i że czekamy z wyjściem z funkcji aż operacja kopiowania zostanie zakończona
		}
	);
	CommandCopyBuffer.copyBuffer(*InSrcBuffer, *InDstBuffer, vk::BufferCopy(0, 0, InSize));
	CommandCopyBuffer.end();
	// 

	vk::SubmitInfo CopyCommandSubmitInfo
	{
		.commandBufferCount = 1,
		.pCommandBuffers = &*CommandCopyBuffer
	};

	GraphicsQueue->submit(CopyCommandSubmitInfo, nullptr);
	GraphicsQueue->waitIdle();  // <- czekamy aż operacja kopiowania zostanie zakończona, 
	//bo inaczej mogłoby dojść do sytuacji że np. 
	// w staging bufferze nadpiszemy dane zanim GPU zdąży je skopiować do dest buffora
	// --- Generalnie w tym miejscu zamiast waitIdle jest również możliwe zastosowanie waitForFences, co daje 
	// możliwość zaplanowania kilku kopiowań jednocześnie, i czekać na zakończenie ich wszystkich, zamiast robić po 
	// kolei

}

uint32_t Vk_Buffers::FindMemoryTypeIndex(uint32_t InTypeFilter, vk::MemoryPropertyFlags InProperties)
{
	vk::PhysicalDeviceMemoryProperties memoryProperties = PhysicalDevice->getMemoryProperties();  // <- pobieramy informacje o tym jakie typy pamięci oferuje GPU

	// memory properties oferuje info o memory types i memory heaps - różne typy pamięci mogą być w różnych "heaps" - czyli jedna może być bezpośrednio w vram, inna w cpu ram - co ma wpływ na wydajność.
	// puki co sprawdzimy tylko memory types

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		//( 1 << i )tu po prostu przesuwamy flagę bitową o 1 w lewo co obrót pętli 0001 -> 0010 -> 0100  // zapis memoryProperties.memoryTypes[i].propertyFlags & InProperties ( bitowe AND ) sprawdza czy zgadza się przynajmniej jedna flaga, 
		// natomiast (memoryProperties.memoryTypes[i].propertyFlags & InProperties) == InProperties sprawdza czy co najmniej wszystkie flagi które wymagam są OK ( dlatego nie może być memoryProperties.memoryTypes[i].propertyFlags == InProperties -> bo w tym przypadku
		// muszą być identyczne. Zatem x & y -> czy x ma co najmniej jedną flagę y, natomiast (x & y ) == y sprawdza czy po zrobieniu x AND y - wyszły wszystkie flagi które są w y ( czyli x & y zwraca flagi które są wspólne dla x i y. )
		if (InTypeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & InProperties) == InProperties)
		{
			return i;
		}
	}

	throw std::runtime_error("No proper memory found");
}

EBufferDataUploadMode Vk_Buffers::PickDataUploadMode()
{
	const vk::PhysicalDeviceMemoryProperties MemoryProperties = PhysicalDevice->getMemoryProperties();
	for (uint32_t i = 0; i < MemoryProperties.memoryTypeCount; ++i)
	{
		const vk::MemoryType& MemoryType = MemoryProperties.memoryTypes[i];

		std::cout
			<< "Memory Type " << i
			<< " Heap: " << MemoryType.heapIndex
			<< " Flags: "
			<< vk::to_string(MemoryType.propertyFlags);

		if ((MemoryType.propertyFlags & IntegratedGPUMemoryFlags) == IntegratedGPUMemoryFlags)
		{
			std::cout << "\n->  Using Direct vertex & index buffers upload mode !\n";
			return EBufferDataUploadMode::Direct;
		}

		std::cout << "\n";		

	}
	std::cout << "   ->  Using Staging vertex & index buffers upload mode !";
	return EBufferDataUploadMode::Staging;
}

Vk_Buffers::BufferStorage Vk_Buffers::GetBufferStorage(vk::BufferUsageFlagBits InBufferUsage)
{
	switch (InBufferUsage)
	{
	case vk::BufferUsageFlagBits::eVertexBuffer:
		return { vertexBuffer, vertexBufferMemory };

	case vk::BufferUsageFlagBits::eIndexBuffer:
		return { indexBuffer, indexBufferMemory };
	}

	throw std::runtime_error("Unsupported buffer type");
}

uint32_t Vk_Buffers::GetBufferMemorySize(vk::BufferUsageFlagBits InBufferUsage)
{
	switch (InBufferUsage)
	{
	case vk::BufferUsageFlagBits::eVertexBuffer:
		return static_cast<uint32_t>(sizeof(Vertex) * vertices.size());

	case vk::BufferUsageFlagBits::eIndexBuffer:
		return static_cast<uint32_t>(sizeof(uint16_t) * indices.size());
	}

	throw std::runtime_error("Unsupported buffer type");
}

const void* Vk_Buffers::FindDataSourceToCopy(vk::BufferUsageFlagBits InBufferUsage)
{
	switch (InBufferUsage)
	{
		case vk::BufferUsageFlagBits::eVertexBuffer:
			return vertices.data();

		case vk::BufferUsageFlagBits::eIndexBuffer:
			return indices.data();
	}

	return nullptr;
}

// musimy jeszcze zbindować Buffers w pipeline


