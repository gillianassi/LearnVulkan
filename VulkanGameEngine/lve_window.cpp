#include "lve_window.h"

namespace lve 
{

	LveWindow::LveWindow(int width, int height, std::string name):
		Width{ width },
		Height{ height },
		WindowName{ name }
	{
		InitWindow();
	}

	LveWindow::~LveWindow()
	{
		glfwDestroyWindow(pWindow);
		glfwTerminate();
	}

	bool LveWindow::ShouldClose()
	{
		return glfwWindowShouldClose(pWindow);
	}

	void LveWindow::InitWindow()
	{
		glfwInit();
		// Don't open an OpenGL context using the GLFW_NO_API hint
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		// Don't resize after creation to handle window resizes ourselves
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		pWindow = glfwCreateWindow(Width, Height, WindowName.c_str(), nullptr, nullptr);
	}

}