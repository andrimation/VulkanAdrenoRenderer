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
		CreateDescriptorSetLayout(InContext);
		CreateGraphicsPipeline(InContext, InWindow, InSwapChain);
	};

	const vk::raii::Pipeline& GetPipeline() const
	{
		return pipeline;
	}

private:

	void CreateDescriptorSetLayout(Vk_Context* InContext); // DescriptorSetLatout pozwala na przekazywanie do shadera globalnych danych np matryc transformacji, czy innych danych które nie są vertexami
	// Generalnie do przekazania DescriprorSet używa się bufferów 
	void CreateGraphicsPipeline(Vk_Context* InContext, WindowGLFW* InWindow, Vk_SwapChain* InSwapChain);
	static std::vector<uint32_t> ReadFile(const std::string& filename);

	[[nodiscard]]
	vk::raii::ShaderModule CreateShaderModule(const std::vector<uint32_t>& shaderBytes,Vk_Context* InContext);

	vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
	vk::raii::PipelineLayout pipelineLayout = nullptr;
	vk::raii::Pipeline pipeline = nullptr;
};