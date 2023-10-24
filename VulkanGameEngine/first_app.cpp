#include "first_app.h"

namespace lve
{
	void FirstApp::run()
	{
		while (!lveWindow.ShouldClose())
		{
			glfwPollEvents();
		}
	}
}


