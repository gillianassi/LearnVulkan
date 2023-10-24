#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

// Little Vulkan Engine
namespace lve 
{

	class LveWindow {

	public:

		LveWindow(int width, int height, std::string name);
		~LveWindow();

		LveWindow(const LveWindow&) = delete;
		LveWindow(LveWindow&&) = delete;
		LveWindow& operator=(const LveWindow&) = delete;

		bool ShouldClose();

	private:

		void InitWindow();

		const int Width;
		const int Height;

		std::string WindowName;
		GLFWwindow* pWindow;

	};
}