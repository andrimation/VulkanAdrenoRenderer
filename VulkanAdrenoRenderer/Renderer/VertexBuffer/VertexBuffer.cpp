#include "VertexBuffer.h"

void Vk_VertexBuffer::CreateVertexBuffer(vk::raii::Device* InDevice, vk::raii::PhysicalDevice* InPhysicalDevice, uint32_t InBufferSize)
{
	vk::BufferCreateInfo bufferInfo{
		.size = InBufferSize,
		.usage = vk::BufferUsageFlagBits::eVertexBuffer, // <- używając bitwise or możemy zrobić że bufor nie będzie wyłącznie jako eVertexBuffer, ale że może mieć kilka zastosowań na raz. 
		.sharingMode = vk::SharingMode::eExclusive       // <- czy bufor będzie używany wyłącznie przez jedną queue czy zakładamy że może być używany przez różne
	};

	vertexBuffer = vk::raii::Buffer(*InDevice, bufferInfo);  // <- w tym momencie mamy utworzony obiekt bufora, ale nie została jeszcze zaalokowana dla niego pamięć

	vk::MemoryRequirements memoryRequirements = vertexBuffer.getMemoryRequirements(); // <- pobieramy wymagania jakie musi spełnić pamięć do przydzielenia dla tego bufora

	vk::MemoryAllocateInfo memoryAllocateInfo{
		.allocationSize = memoryRequirements.size,
		.memoryTypeIndex = FindMemoryTypeIndex(
			InPhysicalDevice,
			memoryRequirements.memoryTypeBits,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			// „chcę pamięć, do której CPU ma dostęp i której cache jest automatycznie spójny z tym, co widzi GPU”.
			// eHostVisible -> pamięć do której CPU ma dostęp
			// eHostCoherent -> pamięć której nie trzeba ręcznie flushMappedMemoryRanges()/invalidateMappedMemoryRanges() po zapisie
			// i odczycie przez CPU
		)
	};

	vertexBufferMemory = vk::raii::DeviceMemory(*InDevice, memoryAllocateInfo);
	// teraz bindujemy zaalokowaną pamięć z vertex bufferem
	vertexBuffer.bindMemory(*vertexBufferMemory, 0);  // jeśli offset nie jest 0 to musi być podzielny przez memoryRequirements.alignment
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

void Vk_VertexBuffer::CopyVerticesToBuffer(uint32_t InMemoryToMapSize)
{
	// mapujemy najpierw pamięć bufora na pamięć dostępną dla CPU
	void* mappedMemory = vertexBufferMemory.mapMemory(0, InMemoryToMapSize);
	memcpy(mappedMemory, &vertices, InMemoryToMapSize);
	vertexBufferMemory.unmapMemory();
}

// musimy jeszcze zbindować VertexBuffer w pipeline


