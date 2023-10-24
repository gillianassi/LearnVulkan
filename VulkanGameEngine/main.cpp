#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "first_app.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() 
{
	lve::FirstApp app{};

	try
	{
		app.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}