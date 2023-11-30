#pragma once

#include <string>
#include <vector>

#include "VLDevice.h"


namespace VulkanLearn {
	// Application layer should be able to configure our pipeline
	struct PipelineConfigInfo {

	};

	class VLPipeline {
	public:
		VLPipeline(VLDevice &InDevice, const std::string& VertFilePath,
			const std::string& FragFilePath, const PipelineConfigInfo& ConfigInfo);
		~VLPipeline() {}

		VLPipeline(const VLPipeline&) = delete;
		VLPipeline(VLPipeline&&) = delete;
		void operator=(const VLPipeline&) = delete;
		void operator=(VLPipeline&&) = delete;

		static PipelineConfigInfo DefaultPipelineConfigInfo(uint32_t Width, uint32_t Height);

	private:

		static std::vector<char> ReadFile(const std::string& FilePath);

		void CreateGraphicsPipeline(const std::string& VertFilePath, 
			const std::string& FragFilePath, const PipelineConfigInfo& ConfigInfo);

		void CreateShaderModule(const std::vector<char>& Code, VkShaderModule* ShaderModule);
	
		// Note:	Memory unsafe, device can be released before the pipeline (Aggregation)
		//			Only use implicitly that our member variable will outline any 
		//			instances it contains
		VLDevice& Device;
		// Handle to our Vulkan pipline object
		VkPipeline GraphicsPipeline;
		VkShaderModule VertShaderModule;
		VkShaderModule FragShaderModule;
	};
}