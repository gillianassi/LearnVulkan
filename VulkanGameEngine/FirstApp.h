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

	std::vector<VulkanLearn::VLModel::Vertex> GetSierpinskiVertices(uint32_t dimension);
	void LoadModels();
	void CreatePipelineLayout();
	void CreatePipeline();
	void CreateCommandBuffers();
	void DrawFrame();

	VLWindow AppWindow{ Width, Height, "Hello Vulkan!" };
	VLDevice AppDevice{ AppWindow };
	VLSwapChain AppSwapChain{ AppDevice , AppWindow.GetExtend() };
	std::unique_ptr<VulkanLearn::VLPipeline> AppPipeline;
	std::unique_ptr<VulkanLearn::VLModel> AppModel;
	VkPipelineLayout PipelineLayout;
	std::vector<VkCommandBuffer> CommandBuffers;


};