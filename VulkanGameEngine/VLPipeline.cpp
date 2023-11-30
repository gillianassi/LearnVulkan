#include "VLPipeline.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace VulkanLearn {

	VLPipeline::VLPipeline(VLDevice& InDevice, const std::string& VertFilePath, 
		const std::string& FragFilePath, const PipelineConfigInfo& ConfigInfo):
		Device(InDevice)
	{
		CreateGraphicsPipeline(VertFilePath, FragFilePath, ConfigInfo);
	}

	PipelineConfigInfo VLPipeline::DefaultPipelineConfigInfo(uint32_t Width, uint32_t Height)
	{
		PipelineConfigInfo ConfigInfo{};
		return ConfigInfo;
	}

	std::vector<char> VLPipeline::ReadFile(const std::string& FilePath)
	{
		// ate:		Start reading at the end of the file
		// binary:	Read the file as binary file(avoid text transformations)
		std::ifstream File{ FilePath, std::ios::ate | std::ios::binary };
		if (!File.is_open()) {
			throw std::runtime_error("Failed to open file: " + FilePath);
		}
		// tellg gives the current position, which was set to the eof using ate
		size_t FileSize = (size_t)File.tellg();
		std::vector<char> Buffer(FileSize);

		// Go to the start of the file and read the file
		File.seekg(0);
		File.read(Buffer.data(), FileSize);

		File.close();
		return Buffer;
	}
	void VLPipeline::CreateGraphicsPipeline(const std::string& VertFilePath, 
		const std::string& FragFilePath, const PipelineConfigInfo& ConfigInfo)
	{
		auto VertShaderCode = ReadFile(VertFilePath);
		auto FragShaderCode = ReadFile(FragFilePath);

		std::cout << "Vertex Shader Code Size: " << VertShaderCode.size() << '\n';
		std::cout << "Fragment Shader Code Size: " << FragShaderCode.size() << '\n';
	}
	void VLPipeline::CreateShaderModule(const std::vector<char>& Code, VkShaderModule* ShaderModule)
	{
		VkShaderModuleCreateInfo CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		CreateInfo.codeSize = Code.size();
		CreateInfo.pCode = reinterpret_cast<const uint32_t*>(Code.data());

		if (vkCreateShaderModule(Device.GetDevice(), &CreateInfo, nullptr, ShaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
	}
}