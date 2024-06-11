#pragma once

#include <memory>
#include <vector>

#include <memory>
#include <vector>

#include "VLWindow.h"
#include "VLPipeline.h"
#include "VLDevice.h"
#include "VLSwapChain.h"
#include "VLModel.h"

using namespace VulkanLearn;
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
	void LoadModels();
	void CreatePipelineLayout();
	void RecreateSwapChain();
	void DrawFrame();
	void RecordCommandBuffer(int imageIndex);
	void CreatePipeline();
	void CreateCommandBuffers();
	void FreeCommandBuffers();

	VLWindow AppWindow{ Width, Height, "Hello Vulkan!" };
	VLDevice AppDevice{ AppWindow };
	std::unique_ptr<VLSwapChain> AppSwapChain;
	std::unique_ptr<VulkanLearn::VLPipeline> AppPipeline;
	std::unique_ptr<VulkanLearn::VLModel> AppModel;
	VkPipelineLayout PipelineLayout;
	std::vector<VkCommandBuffer> CommandBuffers;


};