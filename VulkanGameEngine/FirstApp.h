#pragma once

#include "VLWindow.h"
#include "VLPipeline.h"
#include "VLDevice.h"

namespace VulkanLearn 
{
	class FirstApp {

	public:
		static constexpr int Width = 800;
		static constexpr int Height = 600;

		void run();
	private:
		VLWindow AppWindow{Width, Height, "Hello Vulkan!"};
		VLDevice AppDevice{ AppWindow };
		VLPipeline AppPipeline{AppDevice, "Shaders/TestShader.vert.spv", 
			"Shaders/TestShader.frag.spv", 
			VLPipeline::DefaultPipelineConfigInfo(Width, Height)};
	};
}