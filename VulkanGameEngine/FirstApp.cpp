#include "FirstApp.h"

namespace VulkanLearn
{
	void FirstApp::run()
	{
		while (!AppWindow.ShouldClose())
		{
			glfwPollEvents();
		}
	}
}


