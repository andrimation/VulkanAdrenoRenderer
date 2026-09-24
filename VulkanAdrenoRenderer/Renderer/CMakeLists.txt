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
    "VertexBuffer/VertexFactory.h"
    "VertexBuffer/VertexBuffer.h"
    "VertexBuffer/VertexBuffer.cpp"
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

# ---- Slang shader compilation ----

find_program(
    SLANGC_EXECUTABLE
    NAMES slangc slangc.exe
    HINTS
        "$ENV{VULKAN_SDK}/Bin"
        "$ENV{VULKAN_SDK}/bin"
    REQUIRED
)

set(SLANG_SHADER
    ${CMAKE_CURRENT_SOURCE_DIR}/VkPipeline/shader.slang
)

set(SLANG_SPV
    ${CMAKE_CURRENT_SOURCE_DIR}/VkPipeline/slang.spv
)

add_custom_command(
    OUTPUT ${SLANG_SPV}
    COMMAND ${SLANGC_EXECUTABLE}
        ${SLANG_SHADER}
        -target spirv
        -profile spirv_1_4
        -emit-spirv-directly
        -fvk-use-entrypoint-name
        -entry vertMain
        -entry fragMain
        -o ${SLANG_SPV}
    DEPENDS ${SLANG_SHADER}
    COMMENT "Compiling shader.slang -> slang.spv"
    VERBATIM
)

add_custom_target(VulkanShaders DEPENDS ${SLANG_SPV})

add_dependencies(VulkanRenderer VulkanShaders)