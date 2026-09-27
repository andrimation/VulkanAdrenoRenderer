#include "VertexBuffer.h"
#include "VertexBuffer.h"
#include "VertexBuffer.h"
#include "VertexBuffer.h"
#include "VertexBuffer.h"
#include "VertexBuffer.h"

void Vk_VertexBuffer::CreateVertexBuffer(vk::raii::Device* InDevice, vk::raii::PhysicalDevice* InPhysicalDevice, uint32_t InBufferSize, EBufferDataUploadMode InUploadMode, vk::raii::CommandPool* InCommandPool, vk::raii::Queue* InGraphicsQueue)
{	
	if (InUploadMode == EBufferDataUploadMode::Direct)
	{
		
		CreateUsingDirectBuffer(InDevice,InPhysicalDevice,InBufferSize);	

	}
	else if (InUploadMode == EBufferDataUploadMode::Staging)
	{

		CreateUsingStagingBuffer(InDevice, InPhysicalDevice, InBufferSize, InCommandPool, InGraphicsQueue);

	}
}

void Vk_VertexBuffer::CreateUsingDirectBuffer(vk::raii::Device* InDevice, vk::raii::PhysicalDevice* InPhysicalDevice, uint32_t InBufferSize)
{
	vk::BufferUsageFlagBits DirectBufferUsageFlags = vk::BufferUsageFlagBits::eVertexBuffer;  // <- czyli że będzie źródłem transferu do GPU
	vk::MemoryPropertyFlags DirectBufferMemoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent; // <- pamięć dostępna dla CPU i GPU (?)

	auto [buffer, bufferMemory] = CreateBuffer(InDevice, InPhysicalDevice, InBufferSize, DirectBufferUsageFlags, DirectBufferMemoryProperties);

	vertexBuffer = std::move(buffer);
	vertexBufferMemory = std::move(bufferMemory);

	// Kopiujemy vertices bezpośrednio do wspólnego bufora
	CopyVerticesToBuffer(vertexBufferMemory, vertices.data(), InBufferSize);
}

void Vk_VertexBuffer::CreateUsingStagingBuffer(vk::raii::Device* InDevice, vk::raii::PhysicalDevice* InPhysicalDevice, uint32_t InBufferSize, vk::raii::CommandPool* InCommandPool, vk::raii::Queue* InGraphicsQueue)
{
	// Staging Buffer
	vk::BufferUsageFlagBits StageBufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;  // <- czyli że będzie źródłem transferu do GPU
	vk::MemoryPropertyFlags StageBufferMemoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent; // <- pamięć dostępna dla CPU i GPU (?)

	auto [stagingBuffer, stagingBufferMemory] = CreateBuffer(InDevice, InPhysicalDevice, InBufferSize, StageBufferUsageFlags, StageBufferMemoryProperties);

	// kopiujemy dane do staging buffera
	CopyVerticesToBuffer(stagingBufferMemory,vertices.data(), InBufferSize);


	// Destination buffer
	vk::BufferUsageFlags DestBufferUsageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;  // <- czyli że będzie buforem docelowym, do którego będziemy przenosić dane
	vk::MemoryPropertyFlags DestBufferMemoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal; // <- czyli że pamięć dostępna tylko dla CPU

	// std::tie "rozpakowuje" tuple zwracaną przez CreateBuffer() i przypisuje wyniki bezpośrednio do vertexBuffer,vertexBufferMemory ( czyli nie robimy osbnego move temp itp )
	std::tie(vertexBuffer, vertexBufferMemory) = CreateBuffer(InDevice, InPhysicalDevice, InBufferSize, DestBufferUsageFlags, DestBufferMemoryProperties);

	// kopiujemy staging buffer 
	CopyBuffer(InDevice,&stagingBuffer,&vertexBuffer,InBufferSize, InCommandPool, InGraphicsQueue);
}

// Ta funkcja już nie używa pola buffer w klasie, tylko alokuje nowy i zwraca parę <Buffer, DeviceMemory> - w tym przypadku nie musimy już używać pola buffer w klasie, bo możemy zwrócić parę i przypisać ją do pola w klasie.
std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> Vk_VertexBuffer::CreateBuffer(vk::raii::Device* InDevice, vk::raii::PhysicalDevice* InPhysicalDevice, uint32_t InBufferSize, vk::BufferUsageFlags InBufferUsage, vk::MemoryPropertyFlags InMemoryProperties)
{
	vk::BufferCreateInfo bufferInfo{
		.size = InBufferSize,
		.usage = InBufferUsage, // <- używając bitwise or możemy zrobić że bufor nie będzie wyłącznie jako eVertexBuffer, ale że może mieć kilka zastosowań na raz. 
		.sharingMode = vk::SharingMode::eExclusive       // <- czy bufor będzie używany wyłącznie przez jedną queue czy zakładamy że może być używany przez różne
	};

	vk::raii::Buffer buffer = vk::raii::Buffer(*InDevice, bufferInfo);  // <- w tym momencie mamy utworzony obiekt bufora, ale nie została jeszcze zaalokowana dla niego pamięć)
	vk::MemoryRequirements memoryRequirements = buffer.getMemoryRequirements(); // <- pobieramy wymagania jakie musi spełnić pamięć do przydzielenia dla tego bufora

	vk::MemoryAllocateInfo memoryAllocateInfo{
		.allocationSize = memoryRequirements.size,
		.memoryTypeIndex = FindMemoryTypeIndex(
			InPhysicalDevice,
			memoryRequirements.memoryTypeBits,
			InMemoryProperties	// <- właściwości które pamięć musi spełniać. 	
		)
	};

	vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(*InDevice, memoryAllocateInfo);
	buffer.bindMemory(*bufferMemory, 0);

	return { std::move(buffer),std::move(bufferMemory) };
}

void Vk_VertexBuffer::CopyVerticesToBuffer(vk::raii::DeviceMemory& InDestBufferMemory,const void* InDataSource,uint32_t InMemoryToMapSize)
{
	// mapujemy najpierw pamięć bufora na pamięć dostępną dla CPU
	void* mappedMemory = InDestBufferMemory.mapMemory(0, InMemoryToMapSize);
	memcpy(mappedMemory, InDataSource, InMemoryToMapSize);  // zapamiętać że memcpy kopiować z vector.data()
	InDestBufferMemory.unmapMemory();
}

// Funkcja poniżej służy aby jeden bufor przekopiować do innego
void Vk_VertexBuffer::CopyBuffer(vk::raii::Device* InDevice,vk::raii::Buffer* InSrcBuffer, vk::raii::Buffer* InDstBuffer, uint32_t InSize, vk::raii::CommandPool* InCommandPool, vk::raii::Queue* InGraphicsQueue)
{
	// Data transfer z eTransferSrc do eTransferDst odbywa się za pomocą nagrania command buffera - więc kopiowanie danych ze Staging Bufforu do dest bufforu jest operacją wykonywaną przez GPU - musimy nagrać command, który gpu wykona
	// szczęśliwie nasza główna queue obsługuje commandy związane z transferem. Nagrywamy więc command buffer z poleceniem transferu danych i później robimy submit tego commanda do naszej queue
	vk::CommandBufferAllocateInfo CmdBuffAlocationInfo
	{
		.commandPool = *InCommandPool, // podać command pool i przekazać queue
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};
	
	// zapis poniżej zwraca nam dostęp do jednego command buffera
	vk::raii::CommandBuffer CommandCopyBuffer = std::move(InDevice->allocateCommandBuffers(CmdBuffAlocationInfo).front());
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

	InGraphicsQueue->submit(CopyCommandSubmitInfo, nullptr);
	InGraphicsQueue->waitIdle();  // <- czekamy aż operacja kopiowania zostanie zakończona, 
	//bo inaczej mogłoby dojść do sytuacji że np. 
	// w staging bufferze nadpiszemy dane zanim GPU zdąży je skopiować do dest buffora
	// --- Generalnie w tym miejscu zamiast waitIdle jest również możliwe zastosowanie waitForFences, co daje 
	// możliwość zaplanowania kilku kopiowań jednocześnie, i czekać na zakończenie ich wszystkich, zamiast robić po 
	// kolei

}

uint32_t Vk_VertexBuffer::FindMemoryTypeIndex(vk::raii::PhysicalDevice* InPhysicalDevice, uint32_t InTypeFilter, vk::MemoryPropertyFlags InProperties)
{
	vk::PhysicalDeviceMemoryProperties memoryProperties = InPhysicalDevice->getMemoryProperties();  // <- pobieramy informacje o tym jakie typy pamięci oferuje GPU

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

// musimy jeszcze zbindować VertexBuffer w pipeline


