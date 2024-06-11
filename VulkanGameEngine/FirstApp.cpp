#include "FirstApp.h"

#include <stdexcept>
#include <array>


FirstApp::FirstApp()
{
	LoadModels();
	CreatePipelineLayout();
	RecreateSwapChain();
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
		// TODO:	While resizing, our program will block on PollEvents
		//			To make resizing more smooth, frame should be drawn while resizing is occurring
		glfwPollEvents();
		DrawFrame();
	}

	// Wait until all GPU operations have been completed before ending the run
	vkDeviceWaitIdle(AppDevice.GetDevice());
}

void FirstApp::LoadModels()
{
	std::vector<VulkanLearn::VLModel::Vertex> vertices{
		{{0.0f, -0.5f}, {1, 0, 0}},
		{{0.5f, 0.5f}, {0, 1, 0}},
		{{-0.5f, 0.5f}, {0, 0, 1}}
	};

	AppModel = std::make_unique<VLModel>(AppDevice, vertices);
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

void FirstApp::RecreateSwapChain()
{
	VkExtent2D extent = AppWindow.GetExtent();

	// Let the program wait while a dimension is size less (minimized)
	while (extent.width == 0 || extent.height == 0)
	{
		extent = AppWindow.GetExtent();
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(AppDevice.GetDevice());
	AppSwapChain.reset(nullptr);
	if (AppSwapChain == nullptr) {
		AppSwapChain = std::make_unique<VLSwapChain>(AppDevice, extent);
	}
	else {
		AppSwapChain = std::make_unique<VLSwapChain>(AppDevice, extent, std::move(AppSwapChain));
		if (AppSwapChain->GetImageCount() != CommandBuffers.size()) {
			FreeCommandBuffers();
			CreateCommandBuffers();
		}
	}
	// Note:	Pipeline is Dependant on the swap chain
	// TODO:	Only recreate pipeline if the render pass is not compatible
	//			More info on compatibility: 
	//			https://registry.khronos.org/vulkan/specs/1.1-extensions/html/chap8.html#renderpass-compatibility
	CreatePipeline();
}

void FirstApp::DrawFrame()
{
	uint32_t imageIndex;
	// Note:	Fetch image we should render to next + handle CPU and GPU synchronization surrounding 
	//			double and triple buffering
	VkResult result = AppSwapChain->AcquireNextImage(&imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return;
	}
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	RecordCommandBuffer(imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || AppWindow.WasWindowResized())
	{
		AppWindow.ResetWindowResizedFlag();
		RecreateSwapChain();
		return;
	}

	// Note:	Submit to provided Graphics queue + Handle CPU and GPU synchronization
	//			Command buffer will then be executed
	//			Then the Swap chain will present associated attachment image view to the display
	result = AppSwapChain->SubmitCommandBuffers(&CommandBuffers[imageIndex], &imageIndex);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swap chain image!");
	}
}

void FirstApp::RecordCommandBuffer(int imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(CommandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = AppSwapChain->GetRenderPass();
	renderPassInfo.framebuffer = AppSwapChain->GetFrameBuffer(imageIndex);

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = AppSwapChain->GetSwapChainExtent();

	// In the render pass, we defined our attachments so index 0 as color and 1 as our depth
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	// Note:	VK_SUBPASS_CONTENTS_INLINE signals that the subsequent render pass commands will be 
	//			directly embedded in the primary command buffer itself. + no secondary will be used
	vkCmdBeginRenderPass(CommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(AppSwapChain->GetSwapChainExtent().width);
	viewport.height = static_cast<float>(AppSwapChain->GetSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0, 0}, AppSwapChain->GetSwapChainExtent() };
	vkCmdSetViewport(CommandBuffers[imageIndex], 0, 1, &viewport);
	vkCmdSetScissor(CommandBuffers[imageIndex], 0, 1, &scissor);

	AppPipeline->Bind(CommandBuffers[imageIndex]);
	AppModel->Bind(CommandBuffers[imageIndex]);
	AppModel->Draw(CommandBuffers[imageIndex]);

	vkCmdEndRenderPass(CommandBuffers[imageIndex]);
	if (vkEndCommandBuffer(CommandBuffers[imageIndex]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer!");
	}
}

void FirstApp::CreatePipeline()
{
	assert(AppSwapChain != nullptr && "Cannot create pipeline before swap chain");
	assert(PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	pipelineConfig.RenderPass = AppSwapChain->GetRenderPass();
	pipelineConfig.PipelineLayout = PipelineLayout;
	VLPipeline::DefaultPipelineConfigInfo(pipelineConfig);
	AppPipeline = std::make_unique<VLPipeline>(
		AppDevice,
		"Shaders/TestShader.vert.spv",
		"Shaders/TestShader.frag.spv",
		pipelineConfig);
}

void FirstApp::CreateCommandBuffers()
{
	// TODO:	For simplicity make the command buffers one-to-one in size  to the image count 
	//			to avoid rerecording the command every frame to specify the target output frame
	CommandBuffers.resize(AppSwapChain->GetImageCount());

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
}

void FirstApp::FreeCommandBuffers()
{
	vkFreeCommandBuffers(
		AppDevice.GetDevice(),
		AppDevice.GetCommandPool(),
		static_cast<uint32_t>(CommandBuffers.size()),
		CommandBuffers.data());
	CommandBuffers.clear();
}
