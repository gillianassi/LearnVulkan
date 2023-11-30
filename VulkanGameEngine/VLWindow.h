#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace VulkanLearn 
{

	class VLWindow {

	public:

		VLWindow(int width, int height, std::string name);
		~VLWindow();

		VLWindow(const VLWindow&) = delete;
		VLWindow(VLWindow&&) = delete;
		VLWindow& operator=(const VLWindow&) = delete;

		bool ShouldClose();
		void CreateWindowSufrace(VkInstance Instance, VkSurfaceKHR* Surface);

	private:

		void InitWindow();

		const int Width;
		const int Height;

		std::string WindowName;
		GLFWwindow* pWindow;

	};
}