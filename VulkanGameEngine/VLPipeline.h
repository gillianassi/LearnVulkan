#pragma once

#include <string>
#include <vector>

#include "VLDevice.h"


namespace VulkanLearn
{
	// Application layer should be able to configure our pipeline
	struct PipelineConfigInfo {
		VkViewport Viewport;
		VkRect2D Scissor;
		VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo RasterizationInfo;
		VkPipelineMultisampleStateCreateInfo MultisampleInfo;
		VkPipelineColorBlendAttachmentState ColorBlendAttachment;
		VkPipelineDepthStencilStateCreateInfo DepthStencilInfo;
		VkPipelineLayout PipelineLayout = nullptr;
		VkRenderPass RenderPass = nullptr;
		uint32_t Subpass = 0;
	};

	class VLPipeline {
	public:
		VLPipeline(VLDevice& InDevice, const std::string& VertFilePath,
			const std::string& FragFilePath, const PipelineConfigInfo& ConfigInfo);
		~VLPipeline();

		VLPipeline(const VLPipeline&) = delete;
		VLPipeline(VLPipeline&&) = delete;
		void operator=(const VLPipeline&) = delete;
		void operator=(VLPipeline&&) = delete;

		static PipelineConfigInfo DefaultPipelineConfigInfo(uint32_t width, uint32_t height);
		void Bind(VkCommandBuffer Commandbuffer);

	private:

		static std::vector<char> ReadFile(const std::string& FilePath);

		void CreateGraphicsPipeline(const std::string& VertFilePath,
			const std::string& FragFilePath, const PipelineConfigInfo& ConfigInfo);

		// Note:	pShaderModule is a pointer to a pointer
		//			Needed to create and initialize the variable
		void CreateShaderModule(const std::vector<char>& Code, VkShaderModule* pShaderModule);

		// Note:	Memory unsafe, device can be released before the pipeline (Aggregation)
		//			Only use implicitly that our member variable will outlive any 
		//			instances of the containing class that depend on it
		//			This is not a problem as a pipeline fundamentally needs a device to exist
		VLDevice& Device;
		// Handle to our Vulkan pipeline object
		VkPipeline GraphicsPipeline;
		VkShaderModule VertShaderModule;
		VkShaderModule FragShaderModule;
	};
}