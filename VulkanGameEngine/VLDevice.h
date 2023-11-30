#pragma once

#include "VLWindow.h"

#include <string>
#include <vector>

namespace VulkanLearn {

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR Capabilities;
  std::vector<VkSurfaceFormatKHR> Formats;
  std::vector<VkPresentModeKHR> PresentModes;
};

struct QueueFamilyIndices {
  uint32_t GraphicsFamily;
  uint32_t PresentFamily;
  bool GraphicsFamilyHasValue = false;
  bool PresentFamilyHasValue = false;
  bool IsComplete() { return GraphicsFamilyHasValue && PresentFamilyHasValue; }
};

class VLDevice {
 public:
#ifdef NDEBUG
  const bool EnableValidationLayers = false;
#else
  const bool EnableValidationLayers = true;
#endif

  VLDevice(VLWindow &Window);
  ~VLDevice();

  // Not copyable or movable
  VLDevice(const VLDevice &) = delete;
  void operator=(const VLDevice &) = delete;
  VLDevice(VLDevice &&) = delete;
  VLDevice &operator=(VLDevice &&) = delete;

  VkCommandPool getCommandPool() { return CommandPool; }
  VkDevice GetDevice() { return Device; }
  VkSurfaceKHR GetSurface() { return Surface; }
  VkQueue GetGraphicsQueue() { return GraphicsQueue; }
  VkQueue GetPresentQueue() { return PresentQueue; }

  SwapChainSupportDetails GetSwapChainSupport() 
  { return QuerySwapChainSupport(PhysicalDevice); }
  uint32_t FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties);
  QueueFamilyIndices FindPhysicalQueueFamilies() 
  { return FindQueueFamilies(PhysicalDevice); }
  VkFormat FindSupportedFormat(
      const std::vector<VkFormat> &Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features);

  // Buffer Helper Functions
  void CreateBuffer(
      VkDeviceSize Size,
      VkBufferUsageFlags Usage,
      VkMemoryPropertyFlags Properties,
      VkBuffer &Buffer,
      VkDeviceMemory &BufferMemory);
  VkCommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(VkCommandBuffer CommandBuffer);
  void CopyBuffer(VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize Size);
  void CopyBufferToImage(
      VkBuffer Buffer, VkImage Image, uint32_t Width, uint32_t Height, uint32_t LayerCount);

  void CreateImageWithInfo(
      const VkImageCreateInfo &ImageInfo,
      VkMemoryPropertyFlags Properties,
      VkImage &Image,
      VkDeviceMemory &ImageMemory);

  VkPhysicalDeviceProperties DeviceProperties;

 private:
  void CreateInstance();
  void SetupDebugMessenger();
  void CreateSurface();
  void PickPhysicalDevice();
  void CreateLogicalDevice();
  void CreateCommandPool();

  // helper functions
  bool IsDeviceSuitable(VkPhysicalDevice GetDevice);
  std::vector<const char *> GetRequiredExtensions();
  bool CheckValidationLayerSupport();
  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice GetDevice);
  void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  void HasGflwRequiredInstanceExtensions();
  bool CheckDeviceExtensionSupport(VkPhysicalDevice GetDevice);
  SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice GetDevice);

  VkInstance Instance;
  VkDebugUtilsMessengerEXT DebugMessenger;
  VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
  VLWindow &Window;
  VkCommandPool CommandPool;

  VkDevice Device;
  VkSurfaceKHR Surface;
  VkQueue GraphicsQueue;
  VkQueue PresentQueue;

  const std::vector<const char *> ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

}  // namespace lve