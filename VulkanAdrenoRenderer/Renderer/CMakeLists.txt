# CMakeList.txt : CMake project for VkRenderer, include source and define
# project specific logic here.
#


add_library(VulkanRenderer STATIC
	VulkanRenderer.cpp
	VulkanRenderer.h
    "GLFWWindow/WindowGLFW.cpp"
    "GLFWWindow/WindowGLFW.h"
	"VkContext/VkContext.h" 
	"VkContext/VkContext.cpp"
	"VkSwapChain/VkSwapChain.h"
	"VkSwapChain/VkSwapChain.cpp"
	"VkPipeline/VkPipeline.h"
	"VkPipeline/VkPipeline.cpp"
	"VkCommands/VkCommands.h"
	"VkCommands/VkCommands.cpp"
    "VkSynchronization/VkSynchronization.h"
    "VkSynchronization/VkSynchronization.cpp"
)

target_include_directories(VulkanRenderer 
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(VulkanRenderer
	PUBLIC
		Vulkan::Vulkan
		glfw
)

target_compile_features(VulkanRenderer PUBLIC cxx_std_20)