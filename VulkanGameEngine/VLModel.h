#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>

#include "VLDevice.h"

namespace VulkanLearn
{

	// TODO:	This Model class will quickly run into the max memory allocation limits for complex scenes with many
	//			different types of models.
	//			Solution: Allocate bigger parts of memory and assign parts of them to particular resources.
	//			Possible Reference: https://kylehalladay.com/blog/tutorial/2017/12/13/Custom-Allocators-Vulkan.html

	class VLModel
	{
	public:

		struct Vertex
		{
			glm::vec2 Position;
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
		};

		VLModel(VLDevice& InDevice, const std::vector<Vertex>& Vertices);
		~VLModel();

		VLModel(const VLModel&) = delete;
		VLModel(VLModel&&) = delete;
		VLModel& operator=(const VLModel&) = delete;

		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);

	private:

		void CreateVertexBuffers(const std::vector<Vertex>& Vertices);

		VLDevice& Device;
		VkBuffer VertexBuffer;
		VkDeviceMemory VertexBufferMemory;
		uint32_t VertexCount;
	};
}
