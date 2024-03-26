#include "FirstApp.h"

#include <stdexcept>
#include <array>

FirstApp::FirstApp()
{
	CreatePipelineLayout();
	CreatePipeline();
	CreateCommandBuffers();
}

FirstApp::~FirstApp()
{
	vkDestroyPipelineLayout(AppDevice.GetDevice(), PipelineLayout, nullptr);
}

void FirstApp::run()
{
	while (!AppWindow.ShouldClose())
	{
		glfwPollEvents();
		DrawFrame();
	}

	// Wait until all GPU operations have been completed before ending the run
	vkDeviceWaitIdle(AppDevice.GetDevice());
}

void FirstApp::CreatePipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	// Can be used to send data other than our vertex data to our shaders
	pipelineLayoutInfo.pSetLayouts = nullptr;
	// Can be used to Efficiently send a small amount of data to our shader programs
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
	if (vkCreatePipelineLayout(AppDevice.GetDevice(), &pipelineLayoutInfo, nullptr, &PipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout!");
	}
}

void FirstApp::CreatePipeline()
{
	VulkanLearn::PipelineConfigInfo pipelineConfig = VulkanLearn::VLPipeline::DefaultPipelineConfigInfo(
		AppSwapChain.GetWidth(), AppSwapChain.GetHeight());
	pipelineConfig.RenderPass = AppSwapChain.GetRenderPass();
	pipelineConfig.PipelineLayout = PipelineLayout;
	AppPipeline = std::make_unique<VulkanLearn::VLPipeline>(
		AppDevice,
		"Shaders/TestShader.vert.spv",
		"Shaders/TestShader.frag.spv",
		pipelineConfig);
}

void FirstApp::CreateCommandBuffers()
{
	// TODO:	For simplicity make the command buffers one-to-one in size  to the image count 
	//			to avoid rerecording the command every frame to specify the target output frame
	CommandBuffers.resize(AppSwapChain.GetImageCount());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	// Note:	Primary can be submitted to a queue for execution but cannot be called by other command buffers
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = AppDevice.GetCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t>(CommandBuffers.size());

	if (vkAllocateCommandBuffers(AppDevice.GetDevice(), &allocInfo, CommandBuffers.data()) !=
		VK_SUCCESS) 
	{
		throw std::runtime_error("Failed to allocate command buffers!");
	}

	for (int i = 0; i < CommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(CommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = AppSwapChain.GetRenderPass();
		renderPassInfo.framebuffer = AppSwapChain.GetFrameBuffer(i);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = AppSwapChain.GetSwapChainExtent();

		// In the render pass, we defined our attachments so index 0 as color and 1 as our depth
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// Note:	VK_SUBPASS_CONTENTS_INLINE signals that the subsequent render pass commands will be 
		//			directly embedded in the primary command buffer itself. + no secondary will be used
		vkCmdBeginRenderPass(CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		AppPipeline->Bind(CommandBuffers[i]);
		// Draw 3 vertices in 1 instance
		vkCmdDraw(CommandBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(CommandBuffers[i]);
		if (vkEndCommandBuffer(CommandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer!");
		}
	}
}

void FirstApp::DrawFrame()
{
	uint32_t imageIndex;
	// Note:	Fetch image we should render to next + handle CPU and GPU synchronization surrounding 
	//			double and triple buffering
	VkResult result = AppSwapChain.AcquireNextImage(&imageIndex);
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		// TODO:	Handle this, as it can occur when window is resized
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	// Note:	Submit to provided Graphics queue + Handle CPU and GPU synchronization
	//			Command buffer will then be executed
	//			Then the Swap chain will present associated attachment image view to the display
	result = AppSwapChain.SubmitCommandBuffers(&CommandBuffers[imageIndex], &imageIndex);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swap chain image!");
	}
}


