#include "VkSynchronization.h"

#include "VkSwapChain/VkSwapChain.h"
#include "VkContext/VkContext.h"

void Vk_Synchronization::InitVkSynchronization(Vk_Context* InContext, Vk_SwapChain* InSwapChain, uint32_t MAX_FRAMES_IN_FLIGHT)
{
	assert(getImageCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && drawFences.empty());

	// Czyli zdaje się jest tak: Potrzebujemy tyle renderFinishedSemaphores ile jest obrazów we swap chain
	// natomiast drawFences i getImageCompleteSemaphores, potrzebujemy tyle ile jest MAX_FRAMES_IN_FLIGHT
	// - po 1 nie będziemy odpalać więcej renderów niż jest MAX_FRAMES_IN_FLIGHT. Jednak w momencie gdy pobieramy
	// Image z swapChaina nie wiemy którą klatkę dostaniemy - chodzi po prostu o to, aby klatka którą dostajemy w danym
	// momencie, miała swój, przypisany do siebie renderFinishedSemaphore - niezależnie od rego ile jest Frames in flight.

	for (size_t i = 0; i < InSwapChain->swapChainImages.size(); i++)
	{
		renderFinishedSemaphores.emplace_back(InContext->logicalDevice, vk::SemaphoreCreateInfo());
		
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		drawFences.emplace_back(InContext->logicalDevice, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
		getImageCompleteSemaphores.emplace_back(InContext->logicalDevice, vk::SemaphoreCreateInfo());
	}
}
