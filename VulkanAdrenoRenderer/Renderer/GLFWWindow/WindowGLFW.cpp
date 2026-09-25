#include "WindowGLFW.h"
#include "WindowGLFW.h"

void WindowGLFW::InitWindow(int window_width, int window_height,const char* window_title, void* InRendererPtr, GLFWframebuffersizefun InResizeCallback)
{
	glfwInit();

	// ok, to poniżej działa na zasadzie (1arg: klucz, 2arg: wartosć) 
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // <- tu mówimy glfw żeby nie tworzyło contextu dla OpenGL (glfw było oryginalnie tworzone dla OpenGL)
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);    // <- tu ustawiamy wyłączenie resize okienka ( bo z tym na początek bedzie za dużo jebania )

	window = glfwCreateWindow(window_width, window_height, window_title, nullptr, nullptr);

	// i funkcja poniżej binduje callback z eventem kiedy zmienia się rozmiar okna. Ważne - funkcja którą bindujemy
	// musi być statczyną funkcją ( bo glfw nie wie jak prawidłowo przekazać this )
	// problem polega na tym że z poziomu framebufferResizeCallback chcemy zmienić wartość frameBufferResized. Jako że funkcja jest statyczna, to nie ma dostępu do pól klasy
	// -> ale jest trick - funkcja przyjmuje window, a window pozwala na ustawienie dodatkowego pointera używkownika - a więc przekażemy this ;)
	glfwSetWindowUserPointer(window, InRendererPtr);
	glfwSetFramebufferSizeCallback(window, InResizeCallback);
}

void WindowGLFW::DestroyWindow()
{
	if (window != nullptr)
	{
		glfwDestroyWindow(window);
	}
	glfwTerminate();
}

void WindowGLFW::SetTitle(const std::string& InTitle)
{
	glfwSetWindowTitle(window, InTitle.c_str());
}

