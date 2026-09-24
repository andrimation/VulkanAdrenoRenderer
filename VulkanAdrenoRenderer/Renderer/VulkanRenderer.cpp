#include "VulkanRenderer.h"
#include "VulkanRenderer.h"
#include "VulkanRenderer.h"
#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer()
{

}

void VulkanRenderer::MainLoop()
{
	glfwGetWindowSize(Window.window, &previousWidth, &prewiousHeight);
	while (!glfwWindowShouldClose(Window.window))
	{
		glfwPollEvents();
		DrawFrame(&VulkanContext, &VulkanSwapChain, &VulkanPipeline);
	}
	VulkanContext.logicalDevice.waitIdle();

	Cleanup();
}

// Czwartek - bardzo rozkminić te funkcję.

void VulkanRenderer::DrawFrame(Vk_Context* InContext, Vk_SwapChain* InSwapChain, Vk_Pipeline* InPipeline)
{ // Zanim zaczniemy rysować klatkę, czekamy na fence - fence blokuje CPU - - draw frame jest wywoływane w pętli, więc chcemy żeby zaczekało kiedy
  // na pewno zakończy się renderować                          // vk::True wskazuje że czekamy na wszystkie fences ( w tym przypadku to bez znaczenia bo i tak jest jeden ) ( funkcja może czekać aż wszystkie fences będą signaled, albo jakikolwiek )

	// Uwaga teraz drawFences, presentCompleteSemaphores i commandBuffers zależą od frameIndex a 
	// renderFinishedSemaphores zależy od imageIndex     -> UINT64_MAX to timeout, czyli czekamy (w tym przypadku) w nieskończoność aż fence zostanie zasygnalizowany
	auto fenceResult = InContext->logicalDevice.waitForFences(*VulkanSynchronization.drawFences[VulkanSynchronization.frameIndex], vk::True, UINT64_MAX); // <- wait for fences czeka defacto na rezultat funkcji graphicsQueue.submit(submitInfo, *drawFences[frameIndex]);

	if (fenceResult != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait for fence");
	}

	//logicalDevice.resetFences(*drawFences[frameIndex]);  <- tu wcześniej było reset fences, ale musimy przeniesć je "po" return ( recreateSwapChain(); )

	// Teraz pobieramy wolny Image ze swapChain  -> dostajemy index Image którego możemy użyć ( a result to vk::Result )
	auto [result, imageIndex] = VulkanSwapChain.swapChain.acquireNextImage(UINT64_MAX, *VulkanSynchronization.getImageCompleteSemaphores[VulkanSynchronization.frameIndex], nullptr);

	// imageIndex - wskazuje który image ze swap chain będziemy używać
	// frameIndex - wskazuje która to klatka z Frames_in_flight

	if (result == vk::Result::eErrorOutOfDateKHR || frameBufferResized)
	{
		frameBufferResized = false;
		recreateSwapChain();
		
		return;
	}
	if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
	{
		assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
		throw std::runtime_error("Failed to acquire swap chain image");
	}

	InContext->logicalDevice.resetFences(*VulkanSynchronization.drawFences[VulkanSynchronization.frameIndex]); // <- przeniesienie tutaj wynika z tego, że jak mamy logicalDevice.waitForFences(*drawFences[frameIndex] to czekamy aż fence zostanie zasygnalizowany że
	// "można robić". Jeśli zresetujemy fence i wyjdziemy, to nie wywoła się nigdy funkcja graphicsQueue.submit(submitInfo, *drawFences[frameIndex]);, a własnie jej wykonanie gwarantuje "zasygnalizowanie" fence
	// - waitForFences(..) czeka aż fence będzie zasygnalizowany. Początkowo mnie to zmylało - ale - nasz fence tworzony jest w stanie eSignaled - więc pierwsze wywołanie waitForFences() przechodzi, bo fencje jest eSignaled.
	// dalej robimy reset fences - czyli resetujemy fence i dopiero wykonanie graphicsQueue.submit(submitInfo, *drawFences[frameIndex]) sygnalizuje fence
	// -> jeśli przeniesiemy reset fences po return - to w momencie wczesniejszego wyjścia z funkcji, fence będzie wciąż zasygnalizowany. Jeśli jednak dojedziemy do momentu
	// że resetujemy fence (a więc będzie nie zasygnalizowany), to mamy już pewność że nie wyjdziemy zanim wykonamy graphicsQueue.submit(submitInfo, *drawFences[frameIndex]) - czyli triggerujemy funkcję,
	// która będzie sygnalizować fence.

	// teraz robimy record command buffer  > i używamy image index uzyskanego wyżej
	VulkanCommands.RecordCommandBuffer(imageIndex,VulkanSynchronization.frameIndex,InSwapChain,InPipeline,&VulkanVertexBuffer);

	// Submitting command buffer
	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	const vk::SubmitInfo submitInfo{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*VulkanSynchronization.getImageCompleteSemaphores[VulkanSynchronization.frameIndex], // ten semafor jest użyty swapChain.acquireNextImage(UINT64_MAX, *VulkanSynchronization.  <- będzie czekać na ten semaphore  // jeśli jest więcej semaforów, to każdy demafor odpowiada kolejnemu elementowi w pWaitDstStageMask    
		.pWaitDstStageMask = &waitDestinationStageMask,   // <- to określa na gotowość którego poziomu pipeline chcemy czekać -  a chcemy czekać na gotowość do pisania kolorów
		.commandBufferCount = 1,
		.pCommandBuffers = &*VulkanCommands.commandBuffers[VulkanSynchronization.frameIndex],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &*VulkanSynchronization.renderFinishedSemaphores[imageIndex]  // <- jak będzie zrobione to będzie informować używając tego semaphore
	};


	// fence przekazany tu obejmuje tylko ten submit, ale nie obejmuje przyszłej operacji presentKHR
	VulkanContext.graphicsQueue.submit(submitInfo, *VulkanSynchronization.drawFences[VulkanSynchronization.frameIndex]);  // <- ten drawFence informuje że renderowanie jest zakończone
	// ale nie będzie informować o tym że prezentacja została zakończona
	const vk::PresentInfoKHR presentInfoKHR
	{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*VulkanSynchronization.renderFinishedSemaphores[imageIndex],
		.swapchainCount = 1,
		.pSwapchains = &*VulkanSwapChain.swapChain,
		.pImageIndices = &imageIndex,
	};

	// Ok czyli poroblem polega na tym że graphicsQueue.presentKHR(presentInfoKHR); zostaje odpalone - czeka sobie na renderFinishedSemaphores
	// ale CPU leci dalej. Semafor renderFinishedSemaphores zostaje zresetowany w momencie jak wykona się funkcja która na niego czekała
	// np renderFinishedSemaphores -> signaled -> presentKHR zaczyna wyświetlać -> renderFinishedSemaphores -> unsignaled. Problem polega jednak na tym
	// że CPU poleciał już dale i nie dostaje informacji czy ten konkretny semafor jest wciąż używany czy już nie.
	// - mamy za to inną gwarancję -> jeśli acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex],  ...) zwróci nam obraz, to mamy gwarancję że
	// semafor przypisany do danego obrazu został już skonsumowany i możemy go użyć ( to też oznacza że operacja prezentacji została zakończona )
	result = InContext->graphicsQueue.presentKHR(presentInfoKHR);

	// presentKHR również zwraca info o sukcesie albo nie. JEśli mamy result suboptimal albo OutOfDate to odtwarzamy swap chain
	// ( nie musimy robić tu return, bo to i tak już końcówka drawFrame)
	if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || frameBufferResized)
	{
		frameBufferResized = false;
		recreateSwapChain();
	}
	else
	{
		assert(result == vk::Result::eSuccess);
	}

	VulkanSynchronization.frameIndex = (VulkanSynchronization.frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto appPtr = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
	appPtr->frameBufferResized = true;
}
