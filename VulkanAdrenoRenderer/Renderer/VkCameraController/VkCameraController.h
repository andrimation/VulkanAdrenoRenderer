#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Vk_SceneTransforms;

class Vk_CameraController
{
public:

	void Init(
		GLFWwindow* InWindow,
		Vk_SceneTransforms* InSceneTransforms,
		const glm::vec3& InPosition = glm::vec3(2.0f, 2.0f, 2.0f),
		const glm::vec3& InTarget = glm::vec3(0.0f, 0.0f, 0.0f)
	);

	void Update(float InDeltaTime);

	void SetMoveSpeed(float InMoveSpeed)
	{
		MoveSpeed = InMoveSpeed;
	}

	void SetMouseSensitivity(float InMouseSensitivity)
	{
		MouseSensitivity = InMouseSensitivity;
	}

private:

	void ProcessKeyboard(float InDeltaTime);
	void ProcessMouse();

	void UpdateDirectionVectors();
	void UpdateSceneView();

private:

	GLFWwindow* Window = nullptr;
	Vk_SceneTransforms* SceneTransforms = nullptr;

	glm::vec3 Position = glm::vec3(2.0f, 2.0f, 2.0f);

	glm::vec3 Front = glm::vec3(-1.0f, -1.0f, -1.0f);
	glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);

	// W Twoim rendererze Z jest osią "up".
	glm::vec3 WorldUp = glm::vec3(0.0f, 0.0f, 1.0f);

	float Yaw = 0.0f;
	float Pitch = 0.0f;

	float MoveSpeed = 2.5f;
	float MouseSensitivity = 0.1f;

	double LastMouseX = 0.0;
	double LastMouseY = 0.0;
};
