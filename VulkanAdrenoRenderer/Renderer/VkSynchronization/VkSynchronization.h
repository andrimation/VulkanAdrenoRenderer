#pragma once

#include "../VulkanCommon.h"

class Vk_Context;
class Vk_SwapChain;
class Vk_Pipeline;

class Vk_Synchronization
{
public:

	Vk_Synchronization() {};

	void InitVkSynchronization(Vk_Context* InContext, Vk_SwapChain* InSwapChain, uint32_t MAX_FRAMES_IN_FLIGHT);

	std::vector<vk::raii::Semaphore> getImageCompleteSemaphores;
	std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
	std::vector<vk::raii::Fence> drawFences;
	uint32_t frameIndex = 0;
};