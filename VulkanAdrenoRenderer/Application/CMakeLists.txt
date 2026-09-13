# CMakeList.txt : CMake project for VulkanAdrenoRenderer, include source and define
# project specific logic here.
#

add_library(VulkanApplication STATIC
	VulkanApplication.cpp
	VulkanApplication.h
)

target_link_libraries(VulkanApplication
	PUBLIC
		VulkanRenderer		
)

target_compile_features(VulkanApplication PRIVATE cxx_std_20)

# TODO: Add tests and install targets if needed.
