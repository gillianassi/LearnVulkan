#pragma once

#include "lve_window.h"

namespace lve 
{
	class FirstApp {

	public:
		static constexpr int Width = 800;
		static constexpr int Height = 600;

		void run();
	private:
		LveWindow lveWindow{Width, Height, "Hello Vulkan!"};
	};


}