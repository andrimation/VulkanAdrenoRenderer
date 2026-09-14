#pragma once

#ifndef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#endif

#ifndef VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#endif

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN // czyli w glfw3.h jest #if dedined(GLFW_INCLUDE_VULCAN) -> 
#include <GLFW/glfw3.h>   

inline constexpr bool enableValidationLayers =
#ifdef NDEBUG
false;
#else
true;
#endif

inline constexpr std::array<char const*, 1> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

