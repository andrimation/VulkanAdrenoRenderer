#include "VkCameraController.h"

#include "../VkSceneTransforms/VkSceneTransforms.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>


void Vk_CameraController::Init(
	GLFWwindow* InWindow,
	Vk_SceneTransforms* InSceneTransforms,
	const glm::vec3& InPosition,
	const glm::vec3& InTarget)
{
	assert(InWindow);
	assert(InSceneTransforms);

	Window = InWindow;
	SceneTransforms = InSceneTransforms;

	Position = InPosition;
	const glm::vec3 InitialDirection = InTarget - InPosition;
	assert(glm::length(InitialDirection) > 0.0001f);
	Front = glm::normalize(InitialDirection);

	/*
	 * Wyliczamy początkowy Yaw i Pitch z kierunku kamery.
	 *
	 * Dzięki temu kamera zaczyna dokładnie tak samo jak wcześniej:
	 *
	 * Position = (2, 2, 2)
	 * Target   = (0, 0, 0)
	 */
	Yaw = glm::degrees(
		std::atan2(Front.y, Front.x)
	);

	Pitch = glm::degrees(
		std::asin(
			std::clamp(Front.z, -1.0f, 1.0f)
		)
	);

	UpdateDirectionVectors();

	/*
	 * GLFW_CURSOR_DISABLED powoduje, że kursor nie zatrzymuje się
	 * na krawędzi okna.
	 *
	 * Możemy więc obracać kamerą bez ograniczenia.
	 */
	glfwSetInputMode(
		Window,
		GLFW_CURSOR,
		GLFW_CURSOR_DISABLED
	);

	/*
	 * Jeżeli platforma wspiera raw mouse input,
	 * warto go użyć do sterowania kamerą.
	 */
	if (glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(
			Window,
			GLFW_RAW_MOUSE_MOTION,
			GLFW_TRUE
		);
	}

	glfwGetCursorPos(
		Window,
		&LastMouseX,
		&LastMouseY
	);

	UpdateSceneView();
}


void Vk_CameraController::Update(float InDeltaTime)
{
	assert(Window);
	assert(SceneTransforms);

	ProcessMouse();
	ProcessKeyboard(InDeltaTime);

	UpdateSceneView();
}


void Vk_CameraController::ProcessKeyboard(float InDeltaTime)
{
	// ESC - zamknięcie okna
	if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(Window, GLFW_TRUE);
	}

	const float MovementDelta = MoveSpeed * InDeltaTime;

	// W - do przodu
	if (glfwGetKey(Window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		Position += Front * MovementDelta;
	}

	// S - do tyłu
	if (glfwGetKey(Window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		Position -= Front * MovementDelta;
	}

	// A - w lewo
	if (glfwGetKey(Window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		Position -= Right * MovementDelta;
	}

	// D - w prawo
	if (glfwGetKey(Window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		Position += Right * MovementDelta;
	}

	// W górę
	if (glfwGetKey(Window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
	{
		Position += WorldUp * MovementDelta;
	}

	// W dół
	if (glfwGetKey(Window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
	{
		Position -= WorldUp * MovementDelta;
	}
}


void Vk_CameraController::ProcessMouse()
{
	double MouseX = 0.0;
	double MouseY = 0.0;

	glfwGetCursorPos(
		Window,
		&MouseX,
		&MouseY
	);

	const double MouseDeltaX =
		MouseX - LastMouseX;

	/*
	 * Y odwracamy:
	 *
	 * przesunięcie myszy do góry
	 * -> zwiększenie Pitch
	 */
	const double MouseDeltaY = LastMouseY - MouseY;

	LastMouseX = MouseX;
	LastMouseY = MouseY;

	Yaw -= static_cast<float>(MouseDeltaX) * MouseSensitivity;

	Pitch += static_cast<float>(MouseDeltaY) * MouseSensitivity;

	/*
	 * Nie pozwalamy kamerze przekroczyć +/-90 stopni,
	 * bo wtedy kierunek Right zrobiłby się problematyczny.
	 */
	Pitch = std::clamp(
		Pitch,
		-89.0f,
		89.0f
	);

	UpdateDirectionVectors();
}


void Vk_CameraController::UpdateDirectionVectors()
{
	const float YawRadians = glm::radians(Yaw);
	const float PitchRadians = glm::radians(Pitch);

	glm::vec3 Direction;

	Direction.x =
		std::cos(PitchRadians) *
		std::cos(YawRadians);

	Direction.y =
		std::cos(PitchRadians) *
		std::sin(YawRadians);

	Direction.z =
		std::sin(PitchRadians);

	Front = glm::normalize(Direction);

	/*
	 * cross(Front, WorldUp) daje nam wektor
	 * wskazujący "prawo" względem aktualnego
	 * kierunku patrzenia.
	 */
	Right = glm::normalize(
		glm::cross(Front, WorldUp)
	);
}

void Vk_CameraController::UpdateSceneView()
{
	SceneTransforms->UpdateCameraView(
		Position,
		Front,
		WorldUp
	);
}