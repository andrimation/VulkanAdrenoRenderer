# pragma once

#include "../VulkanCommon.h"

class Vk_Context;
class WindowGLFW;
class Vk_SwapChain;

class Vk_Pipeline
{
public:

	Vk_Pipeline() {};

	void InitVkPipeline(Vk_Context* InContext, WindowGLFW* InWindow, Vk_SwapChain* InSwapChain)
	{
		CreateGraphicsPipeline(InContext, InWindow, InSwapChain);
	};

	const vk::raii::Pipeline& GetPipeline() const
	{
		return pipeline;
	}

private:

	void CreateGraphicsPipeline(Vk_Context* InContext, WindowGLFW* InWindow, Vk_SwapChain* InSwapChain);
	static std::vector<uint32_t> ReadFile(const std::string& filename);

	[[nodiscard]]
	vk::raii::ShaderModule CreateShaderModule(const std::vector<uint32_t>& shaderBytes,Vk_Context* InContext);

	vk::raii::PipelineLayout pipelineLayout = nullptr;
	vk::raii::Pipeline pipeline = nullptr;
};