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
		bool WasWindowResized() { return FrameBufferResized; }
		VkExtent2D GetExtent() { return { static_cast<uint32_t>(Width), static_cast<uint32_t>(Height) }; }

		void CreateWindowSufrace(VkInstance Instance, VkSurfaceKHR* Surface);
		void ResetWindowResizedFlag() { FrameBufferResized = false; }

	private:

		void InitWindow();
		static void FrameBufferResizedCallback(GLFWwindow* Window, int width, int height);

		int Width;
		int Height;
		bool FrameBufferResized = false;

		std::string WindowName;
		GLFWwindow* pWindow;

	};
}