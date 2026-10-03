#include "VkSceneTransforms.h"

void Vk_SceneTransforms::InitVk_SceneTransforms(uint32_t SwapChainExtentWidth, uint32_t SwapChainExtentHeight)
{
	CameraObject.model = glm::mat4(1.0f);

	CameraObject.view = glm::lookAt(
		glm::vec3(2.0f, 2.0f, 2.0f),  // cameraPosition -> czyli gdzie znajduje się camera
		glm::vec3(0.0f, 0.0f, 0.0f),  // targetPosition -> czyli gdzie znajduje się to na co kamera patrzy
		glm::vec3(0.0f, 0.0f, 1.0f)   // upDirection    -> czyli w którą stronę jest "góra"
	);

	CameraObject.projection = glm::perspective(
		glm::radians(45.0f),
		static_cast<float>(SwapChainExtentWidth) / static_cast<float>(SwapChainExtentHeight),
		0.1f,
		10.0f
	);

	// OpenGL i Vulkan mają różne konwencje współrzędnych ekranu ( odwrócenie osi y ) 
	// CameraObject.projection[1][1] *= -1;   UWAGA, to zostanie rozwiązane w commandBuffer.setViewport - nadajemy
	// mu negatywną wartość dla wysokości !!
	// commandBuffer.setViewport(0, vk::Viewport(0.0f, static_cast<float>(swapChainExtent.height), static_cast<float>(swapChainExtent.width), -static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
	// CameraObject.projection[1][1] *= -1; -> to rozwiązanie skutkowałoby koniecznością flipowania fejsów
}

void Vk_SceneTransforms::UpdateCameraView(const glm::vec3& InPosition, const glm::vec3& InTarget, const glm::vec3& InUpDirection)
{
	CameraObject.view = glm::lookAt(
		InPosition,
		InPosition + InTarget,
		InUpDirection
	);
}
