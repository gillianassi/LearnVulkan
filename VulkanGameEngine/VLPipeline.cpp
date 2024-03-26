#include "VLPipeline.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace VulkanLearn {

	VLPipeline::VLPipeline(VLDevice& InDevice, const std::string& VertFilePath,
		const std::string& FragFilePath, const PipelineConfigInfo& ConfigInfo) :
		Device(InDevice)
	{
		CreateGraphicsPipeline(VertFilePath, FragFilePath, ConfigInfo);
	}

	VLPipeline::~VLPipeline()
	{
		vkDestroyShaderModule(Device.GetDevice(), VertShaderModule, nullptr);
		vkDestroyShaderModule(Device.GetDevice(), FragShaderModule, nullptr);
		vkDestroyPipeline(Device.GetDevice(), GraphicsPipeline, nullptr);
	}

	PipelineConfigInfo VLPipeline::DefaultPipelineConfigInfo(uint32_t width, uint32_t height)
	{
		PipelineConfigInfo configInfo{};

		configInfo.InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

		// Tell the Input Assembler how to interpret
		// By our default, every pair of 3 vertices's represent a triangle
		configInfo.InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		// This would be used to break up topologies like a triangle strips
		configInfo.InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// transformation between pipeline's output and target image
		configInfo.Viewport.x = 0.f;
		configInfo.Viewport.y = 0.f;
		configInfo.Viewport.width = static_cast<float>(width);
		configInfo.Viewport.height = static_cast<float>(height);
		// depth transforms the z component of glPosition
		configInfo.Viewport.minDepth = 0.f;
		configInfo.Viewport.maxDepth = 1.f;

		// Instead of squishing and stretching like the viewport, 
		// scissor  cuts out any pixels outside of the scissor rectangle
		configInfo.Scissor.offset = { 0, 0 };
		configInfo.Scissor.extent = { width, height };

		configInfo.RasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		// Would clamp values between 0 and 1
		configInfo.RasterizationInfo.depthClampEnable = VK_FALSE;
		// Would discard all primitives before rasterization
		configInfo.RasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		configInfo.RasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		configInfo.RasterizationInfo.lineWidth = 1.0f;
		// Discard triangles based on their winding order
		configInfo.RasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		configInfo.RasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		// Used to alter depth values by adding a cte value or a factor of the fragment's slope
		configInfo.RasterizationInfo.depthBiasEnable = VK_FALSE;
		configInfo.RasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
		configInfo.RasterizationInfo.depthBiasClamp = 0.0f;           // Optional
		configInfo.RasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

		configInfo.MultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		configInfo.MultisampleInfo.sampleShadingEnable = VK_FALSE;
		configInfo.MultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		configInfo.MultisampleInfo.minSampleShading = 1.0f;           // Optional
		configInfo.MultisampleInfo.pSampleMask = nullptr;             // Optional
		configInfo.MultisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
		configInfo.MultisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

		// Define how we control colors in our frame buffer
		configInfo.ColorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		configInfo.ColorBlendAttachment.blendEnable = VK_FALSE;
		configInfo.ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		configInfo.ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		configInfo.ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		configInfo.ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		configInfo.ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		configInfo.ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

		configInfo.DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
		configInfo.DepthStencilInfo.depthWriteEnable = VK_TRUE;
		configInfo.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.DepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		configInfo.DepthStencilInfo.minDepthBounds = 0.0f;  // Optional
		configInfo.DepthStencilInfo.maxDepthBounds = 1.0f;  // Optional
		configInfo.DepthStencilInfo.stencilTestEnable = VK_FALSE;
		configInfo.DepthStencilInfo.front = {};  // Optional
		configInfo.DepthStencilInfo.back = {};   // Optional

		return configInfo;
	}

	void VLPipeline::Bind(VkCommandBuffer Commandbuffer)
	{
		// Note:	No need to check GraphicsPipeline, as it must have been properly created at initialization
		vkCmdBindPipeline(Commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);
	}

	std::vector<char> VLPipeline::ReadFile(const std::string& FilePath)
	{
		// ate:		Start reading at the end of the file
		// binary:	Read the file as binary file(avoid text transformations)
		std::ifstream file{ FilePath, std::ios::ate | std::ios::binary };
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file: " + FilePath);
		}
		// tellg gives the current position, which was set to the eof using ate
		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		// Go to the start of the file and read the file
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}
	void VLPipeline::CreateGraphicsPipeline(const std::string& VertFilePath,
		const std::string& FragFilePath, const PipelineConfigInfo& ConfigInfo)
	{
		assert(ConfigInfo.PipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline::"
			&& "no pipeline layout provided in the config info");
		assert(ConfigInfo.RenderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline::"
			&& "no render pass provided in the config info");

		std::vector<char> vertShaderCode = ReadFile(VertFilePath);
		std::vector<char> fragShaderCode = ReadFile(FragFilePath);

		CreateShaderModule(vertShaderCode, &VertShaderModule);
		CreateShaderModule(fragShaderCode, &FragShaderModule);

		VkPipelineShaderStageCreateInfo shaderStages[2];
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		// state that this is for the vertex shader
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = VertShaderModule;
		// name of our entry function
		shaderStages[0].pName = "main";
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		// intended to customize shader functionality
		shaderStages[0].pSpecializationInfo = nullptr;

		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = FragShaderModule;
		shaderStages[1].pName = "main";
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;

		// Describe how to interpret the vertex buffer data 
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		// Currently using hard coded data -> not providing any data
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;


		// here you could define multiple view ports and scissors
		VkPipelineViewportStateCreateInfo viewportInfo{};
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.viewportCount = 1;
		viewportInfo.pViewports = &ConfigInfo.Viewport;
		viewportInfo.scissorCount = 1;
		viewportInfo.pScissors = &ConfigInfo.Scissor;
		viewportInfo.pNext = nullptr;
		viewportInfo.flags = 0;

		VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
		colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendInfo.logicOpEnable = VK_FALSE;
		colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		colorBlendInfo.attachmentCount = 1;
		colorBlendInfo.pAttachments = &ConfigInfo.ColorBlendAttachment;
		colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &ConfigInfo.InputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pRasterizationState = &ConfigInfo.RasterizationInfo;
		pipelineInfo.pMultisampleState = &ConfigInfo.MultisampleInfo;
		pipelineInfo.pColorBlendState = &colorBlendInfo;
		pipelineInfo.pDepthStencilState = &ConfigInfo.DepthStencilInfo;
		// Configure line width or viewport dynamically without recreating the pipeline
		pipelineInfo.pDynamicState = nullptr;	// optional
		pipelineInfo.layout = ConfigInfo.PipelineLayout;
		pipelineInfo.renderPass = ConfigInfo.RenderPass;
		pipelineInfo.subpass = ConfigInfo.Subpass;
		// Useful for optimizing performance
		// Deriving from an existing graphics pipeline can be less expensive
		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		// Note:	pipelineCache (now VK_NULL_HANDLE) could be used as a performance opimisation
		if (vkCreateGraphicsPipelines(Device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo,
			nullptr, &GraphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline");
		}
	}
	void VLPipeline::CreateShaderModule(const std::vector<char>& Code, VkShaderModule* pShaderModule)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = Code.size();
		// Note:	Although int32_t and char are not the same size, std:vector ensures that the data satisfies 
		//			the worst case alignment. This would not have worked with a char array!
		createInfo.pCode = reinterpret_cast<const uint32_t*>(Code.data());

		if (vkCreateShaderModule(Device.GetDevice(), &createInfo, nullptr, pShaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
	}
}