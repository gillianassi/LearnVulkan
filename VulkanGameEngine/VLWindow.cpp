#include "VLWindow.h"

#include <stdexcept>

namespace VulkanLearn 
{

	VLWindow::VLWindow(int width, int height, std::string name):
		Width{ width },
		Height{ height },
		WindowName{ name }
	{
		InitWindow();
	}

	VLWindow::~VLWindow()
	{
		glfwDestroyWindow(pWindow);
		glfwTerminate();
	}

	bool VLWindow::ShouldClose()
	{
		return glfwWindowShouldClose(pWindow);
	}

	void VLWindow::CreateWindowSufrace(VkInstance Instance, VkSurfaceKHR* Surface)
	{
		// nullptr for the allocator callback
		if (glfwCreateWindowSurface(Instance, pWindow, nullptr, Surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Window Surface");
		}
	}

	void VLWindow::InitWindow()
	{
		glfwInit();
		// Don't open an OpenGL context using the GLFW_NO_API hint
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		pWindow = glfwCreateWindow(Width, Height, WindowName.c_str(), nullptr, nullptr);
		// Pair our glfw window object with an arbitrary pointer value
		glfwSetWindowUserPointer(pWindow, this);
		glfwSetFramebufferSizeCallback(pWindow, FrameBufferResizedCallback);
	}

	void VLWindow::FrameBufferResizedCallback(GLFWwindow* Window, int width, int height)
	{
		VLWindow* ActiveWindow = reinterpret_cast<VLWindow*>(glfwGetWindowUserPointer(Window));
		ActiveWindow->FrameBufferResized = true;
		ActiveWindow->Width = width;
		ActiveWindow->Height = height;
	}

}