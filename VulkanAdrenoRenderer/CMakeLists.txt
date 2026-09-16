# CMakeList.txt : CMake project for VulkanAdrenoRenderer, include source and define
# project specific logic here.
#

# Add source to this project's executable.

#Dodajemy biblioteki
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

# poniżej ważna jest kolejność ! Application includuje renderer więc renderer musi już istnieć !
add_subdirectory ("Renderer")
add_subdirectory ("Application")


add_executable (VulkanAdrenoRenderer "main.cpp")

target_link_libraries(VulkanAdrenoRenderer
    PRIVATE
        VulkanApplication
)

add_custom_command(
    TARGET VulkanAdrenoRenderer
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:glfw>"
        "$<TARGET_FILE_DIR:VulkanAdrenoRenderer>"
)

add_custom_command(
    TARGET VulkanAdrenoRenderer
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/Renderer/VkPipeline/slang.spv"
        "$<TARGET_FILE_DIR:VulkanAdrenoRenderer>"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET VulkanAdrenoRenderer PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.


