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
};
