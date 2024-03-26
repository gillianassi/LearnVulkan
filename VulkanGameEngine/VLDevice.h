#pragma once

#include "VLWindow.h"

#include <optional>
#include <string>
#include <vector>

namespace VulkanLearn {

	struct SwapChainSupportDetails {
	public:
		// Min/max number of images in swap chain, min/max width and height of images
		VkSurfaceCapabilitiesKHR Capabilities;
		// pixel format, color space
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentationModes;
	};

	struct QueueFamilyIndices {
	public:
		bool IsComplete() { return GraphicsFamily.has_value() && PresentationFamily.has_value(); }

		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentationFamily;
	};

	class VLDevice {
	public:
		VLDevice(VLWindow& Window);
		~VLDevice();

		// Not copyable or movable
		VLDevice(const VLDevice&) = delete;
		void operator=(const VLDevice&) = delete;
		VLDevice(VLDevice&&) = delete;
		VLDevice& operator=(VLDevice&&) = delete;

		VkCommandPool GetCommandPool() { return CommandPool; }
		VkDevice GetDevice() { return Device; }
		VkSurfaceKHR GetSurface() { return Surface; }
		VkQueue GetGraphicsQueue() { return GraphicsQueue; }
		VkQueue GetPresentQueue() { return PresentationQueue; }

		SwapChainSupportDetails GetSwapChainSupport()
		{
			return QuerySwapChainSupport(PhysicalDevice);
		}
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		QueueFamilyIndices FindPhysicalQueueFamilies()
		{
			return FindQueueFamilies(PhysicalDevice);
		}
		VkFormat FindSupportedFormat(
			const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags Features);

		// Buffer Helper Functions
		void CreateBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkBuffer& Buffer,
			VkDeviceMemory& BufferMemory);
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CopyBufferToImage(
			VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

		void CreateImageWithInfo(
			const VkImageCreateInfo& ImageInfo,
			VkMemoryPropertyFlags properties,
			VkImage& Image,
			VkDeviceMemory& ImageMemory);

#ifdef NDEBUG
		const bool EnableValidationLayers = false;
#else
		const bool EnableValidationLayers = true;
#endif

		VkPhysicalDeviceProperties DeviceProperties;

	private:
		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateCommandPool();

		// helper functions
		bool IsDeviceSuitable(VkPhysicalDevice getDevice);
		std::vector<const char*> GetRequiredExtensions();
		bool CheckValidationLayerSupport();
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice getDevice);
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void HasGflwRequiredInstanceExtensions();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice getDevice);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice getDevice);

		VkInstance Instance;
		VkDebugUtilsMessengerEXT DebugMessenger;
		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		VLWindow& Window;
		VkCommandPool CommandPool;

		VkDevice Device;
		VkSurfaceKHR Surface;
		VkQueue GraphicsQueue;
		VkQueue PresentationQueue;

		const std::vector<const char*> ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};

}  // namespace lve