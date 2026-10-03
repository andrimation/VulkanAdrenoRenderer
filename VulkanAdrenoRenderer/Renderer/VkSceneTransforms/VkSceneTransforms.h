#pragma once

#include "../VulkanCommon.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "chrono"

struct SceneTransformMatrices
{
	//glm::vec2 foo;	
	//alignas(16) glm::mat4 model;   // vec2 jest zalignowane do 8, ale żeby to działało z shaderem, wszustko musi być zalignowane to pamięci podzielnej przez największy z elementów
	// więc danie alignas przed mat4 powoduje że cała struktura będzie zalignowana do 16 - czyli do vec2 dodaje padding 8 czyli [vec2][8][mat4 ( pamięć podzielna przez 16 )]
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};

class Vk_SceneTransforms
{
public:
	Vk_SceneTransforms() = default;

	void InitVk_SceneTransforms(uint32_t SwapChainExtentWidth, uint32_t SwapChainExtentHeight);
	void UpdateCameraView(const glm::vec3& InPosition, const glm::vec3& InTarget, const glm::vec3& InUpDirection);

	SceneTransformMatrices* GetCameraObject() { return &CameraObject; };
	// Tu dodać update transforms -
	// mieć jeden obiekt SceneTransformMatrices i updejtować go co klatkę, zamiast tworzyć nowy
	// Kopiować do bufora obiekt tylko jeśli się zmienił - nie kopiować jeśli nie było zmiany

	// - zaimplementować obsługę strzałek do poruszania się i +/- do zwiększania prędkości 
private:
	SceneTransformMatrices CameraObject;
};
