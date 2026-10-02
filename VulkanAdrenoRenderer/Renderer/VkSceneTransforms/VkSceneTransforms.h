#pragma once

#include "../VulkanCommon.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "chrono"

struct SceneTransformMatrices
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};

class Vk_SceneTransforms
{
public:
	Vk_SceneTransforms() = default;

	void InitVk_SceneTransforms(uint32_t SwapChainExtentWidth, uint32_t SwapChainExtentHeight);

	SceneTransformMatrices* GetCameraObject() { return &CameraObject; };
	// Tu dodać update transforms -
	// mieć jeden obiekt SceneTransformMatrices i updejtować go co klatkę, zamiast tworzyć nowy
	// Kopiować do bufora obiekt tylko jeśli się zmienił - nie kopiować jeśli nie było zmiany

	// - zaimplementować obsługę strzałek do poruszania się i +/- do zwiększania prędkości 
private:
	SceneTransformMatrices CameraObject;
};
