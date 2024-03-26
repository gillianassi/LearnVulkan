#pragma once

#include <memory>
#include <vector>

#include "VLWindow.h"
#include "VLPipeline.h"
#include "VLDevice.h"
#include "VLSwapChain.h"

class FirstApp {

public:

	FirstApp();
	~FirstApp();

	FirstApp(const FirstApp&) = delete;
	FirstApp(FirstApp&&) = delete;
	FirstApp& operator=(const FirstApp&) = delete;

	void run();

	static constexpr int Width = 800;
	static constexpr int Height = 600;

private:

	void CreatePipelineLayout();
	void CreatePipeline();
	void CreateCommandBuffers();
	void DrawFrame();

	VulkanLearn::VLWindow AppWindow{ Width, Height, "Hello Vulkan!" };
	VulkanLearn::VLDevice AppDevice{ AppWindow };
	VulkanLearn::VLSwapChain AppSwapChain{ AppDevice , AppWindow.GetExtend() };
	std::unique_ptr<VulkanLearn::VLPipeline> AppPipeline;
	VkPipelineLayout PipelineLayout;
	std::vector<VkCommandBuffer> CommandBuffers;
};