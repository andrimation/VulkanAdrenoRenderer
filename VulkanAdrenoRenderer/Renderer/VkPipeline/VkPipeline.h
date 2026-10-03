# pragma once

#include "../VulkanCommon.h"

class Vk_Context;
class WindowGLFW;
class Vk_SwapChain;

class Vk_Pipeline
{
public:

	Vk_Pipeline() {};

	void InitVkPipeline(Vk_Context* InContext, WindowGLFW* InWindow, Vk_SwapChain* InSwapChain, vk::raii::DescriptorSetLayout* InDescriptorSetLayout)
	{
		Context = InContext;
		Window = InWindow;
		SwapChain = InSwapChain;
		descriptorSetLayout = InDescriptorSetLayout;

		CreateGraphicsPipeline();
	};

	vk::raii::Pipeline* GetPipeline()
	{
		return &pipeline;
	}

	vk::raii::PipelineLayout* GetPipelineLayout()
	{
		return &pipelineLayout;
	}

private:
	void CreateGraphicsPipeline();
	static std::vector<uint32_t> ReadFile(const std::string& filename);

	[[nodiscard]]
	vk::raii::ShaderModule CreateShaderModule(const std::vector<uint32_t>& shaderBytes);

	Vk_Context* Context;
	WindowGLFW* Window;
	Vk_SwapChain* SwapChain;

	vk::raii::DescriptorSetLayout* descriptorSetLayout;
	vk::raii::PipelineLayout pipelineLayout = nullptr;
	vk::raii::Pipeline pipeline = nullptr;
};