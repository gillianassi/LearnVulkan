#include "VLModel.h"

#include <cassert>
#include <cstring>

namespace VulkanLearn
{
	std::vector<VkVertexInputAttributeDescription> VLModel::Vertex::GetAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = 0;
		return attributeDescriptions;
	}

	std::vector<VkVertexInputBindingDescription> VLModel::Vertex::GetBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);;
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	VLModel::VLModel(VLDevice& InDevice, const std::vector<Vertex>& Vertices):
		Device(InDevice)
	{
		CreateVertexBuffers(Vertices);
	}

	VLModel::~VLModel()
	{
		vkDestroyBuffer(Device.GetDevice(), VertexBuffer, nullptr);
		vkFreeMemory(Device.GetDevice(), VertexBufferMemory, nullptr);
	}

	void VLModel::Bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { VertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		// Record to our command buffer to bind 1 vertex buffer starting at 0. 
		// Note:	For multiple bindings, add additional elements to these arrays.
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}
	
	void VLModel::Draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, VertexCount, 1, 0, 0);
	}

	void VLModel::CreateVertexBuffers(const std::vector<Vertex>& Vertices)
	{
		VertexCount = static_cast<uint32_t>(Vertices.size());
		assert(VertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(Vertices[0]) * VertexCount;

		// VK_BUFFER_USAGE_VERTEX_BUFFER_BIT:	Buffer will be used to hold vertex input data
		// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT:	Allocated memory should be accessible from our host (CPU)
		//										This way, we can write to the device memory
		// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:	Keep host and device memory regents consistent with each other
		Device.CreateBuffer(
			bufferSize, 
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			VertexBuffer, 
			VertexBufferMemory);

		// Create a region of host memory mapped to device memory + Point to the beginning  of the mapped memory range
		void* data;
		vkMapMemory(Device.GetDevice(), VertexBufferMemory, 0, bufferSize, 0, &data);

		// Take Vertices data and copy into the Host-Mapped memory region
		// Note:	Without VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vkFlushMappedMemoryRanges is required to propagate 
		//			changes from host to device.
		memcpy(data, Vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(Device.GetDevice(), VertexBufferMemory);
	}
}
