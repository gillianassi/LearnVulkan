#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "FirstApp.h"
#include "SierpinskiTriangleApp.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() 
{
	// Uncomment the app you want to see
	FirstApp app{};
	//SierpinskiTriangleApp app{};

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