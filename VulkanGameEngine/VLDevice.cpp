#include "VLDevice.h"

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

namespace VulkanLearn 
{

// local callback functions
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) 
{
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance Instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) 
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      Instance,
      "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) 
  {
    return func(Instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else 
  {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance Instance,
    VkDebugUtilsMessengerEXT DebugMessenger,
    const VkAllocationCallbacks *pAllocator) 
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      Instance,
      "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) 
  {
    func(Instance, DebugMessenger, pAllocator);
  }
}

// class member functions
VLDevice::VLDevice(VLWindow &Window) : Window{Window} 
{
  // Initialize Vulkan library -> connection between our application and Vulkan
  CreateInstance();
  // Setup validation layers to check for errors (only when debugging)
  SetupDebugMessenger();
  // Connection between our Window and Vulkan
  CreateSurface();
  // Graphics device in our system capable of working with the Vulkan API
  PickPhysicalDevice();
  // Select what features of our physical device we will use
  CreateLogicalDevice();
  // Setup Command Pool for Command Buffer allocation
  CreateCommandPool();
}

VLDevice::~VLDevice() 
{
  vkDestroyCommandPool(Device, CommandPool, nullptr);
  vkDestroyDevice(Device, nullptr);

  if (EnableValidationLayers) 
  {
    DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
  }

  vkDestroySurfaceKHR(Instance, Surface, nullptr);
  vkDestroyInstance(Instance, nullptr);
}

void VLDevice::CreateInstance() 
{
  if (EnableValidationLayers && !CheckValidationLayerSupport()) 
  {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "LittleVulkanEngine App";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  auto extensions = GetRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  if (EnableValidationLayers) 
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
    createInfo.ppEnabledLayerNames = ValidationLayers.data();

    PopulateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
  } else 
  {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  if (vkCreateInstance(&createInfo, nullptr, &Instance) != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance!");
  }

  HasGflwRequiredInstanceExtensions();
}

void VLDevice::PickPhysicalDevice() 
{
  uint32_t DeviceCount = 0;
  vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr);
  if (DeviceCount == 0) 
  {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }
  std::cout << "Device count: " << DeviceCount << std::endl;
  std::vector<VkPhysicalDevice> Devices(DeviceCount);
  vkEnumeratePhysicalDevices(Instance, &DeviceCount, Devices.data());

  for (const auto &GetDevice : Devices) 
  {
    if (IsDeviceSuitable(GetDevice)) 
    {
      PhysicalDevice = GetDevice;
      break;
    }
  }

  if (PhysicalDevice == VK_NULL_HANDLE) 
  {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  vkGetPhysicalDeviceProperties(PhysicalDevice, &DeviceProperties);
  std::cout << "physical device: " << DeviceProperties.deviceName << std::endl;
}

void VLDevice::CreateLogicalDevice() 
{
  QueueFamilyIndices Indices = FindQueueFamilies(PhysicalDevice);

  std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
  std::set<uint32_t> UniqueQueueFamilies = {Indices.GraphicsFamily, Indices.PresentFamily};

  float QueuePriority = 1.0f;
  for (uint32_t QueueFamily : UniqueQueueFamilies) 
  {
    VkDeviceQueueCreateInfo QueueCreateInfo = {};
    QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    QueueCreateInfo.queueFamilyIndex = QueueFamily;
    QueueCreateInfo.queueCount = 1;
    QueueCreateInfo.pQueuePriorities = &QueuePriority;
    QueueCreateInfos.push_back(QueueCreateInfo);
  }

  VkPhysicalDeviceFeatures DeviceFeatures = {};
  DeviceFeatures.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo CreateInfo = {};
  CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
  CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();

  CreateInfo.pEnabledFeatures = &DeviceFeatures;
  CreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
  CreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

  // might not really be necessary anymore because device specific validation layers
  // have been deprecated
  if (EnableValidationLayers) 
  {
    CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
    CreateInfo.ppEnabledLayerNames = ValidationLayers.data();
  } else 
  {
    CreateInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(PhysicalDevice, &CreateInfo, nullptr, &Device) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  vkGetDeviceQueue(Device, Indices.GraphicsFamily, 0, &GraphicsQueue);
  vkGetDeviceQueue(Device, Indices.PresentFamily, 0, &PresentQueue);
}

void VLDevice::CreateCommandPool() 
{
  QueueFamilyIndices QueueFamilyIndices = FindPhysicalQueueFamilies();

  VkCommandPoolCreateInfo PoolInfo = {};
  PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  PoolInfo.queueFamilyIndex = QueueFamilyIndices.GraphicsFamily;
  PoolInfo.flags =
      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(Device, &PoolInfo, nullptr, &CommandPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
  }
}

void VLDevice::CreateSurface() { Window.CreateWindowSufrace(Instance, &Surface); }

bool VLDevice::IsDeviceSuitable(VkPhysicalDevice GetDevice) 
{
  QueueFamilyIndices Indices = FindQueueFamilies(GetDevice);

  bool ExtensionsSupported = CheckDeviceExtensionSupport(GetDevice);

  bool SwapChainAdequate = false;
  if (ExtensionsSupported) 
  {
    SwapChainSupportDetails SwapChainSupport = QuerySwapChainSupport(GetDevice);
    SwapChainAdequate = !SwapChainSupport.Formats.empty() && !SwapChainSupport.PresentModes.empty();
  }

  VkPhysicalDeviceFeatures SupportedFeatures;
  vkGetPhysicalDeviceFeatures(GetDevice, &SupportedFeatures);

  return Indices.IsComplete() && ExtensionsSupported && SwapChainAdequate &&
         SupportedFeatures.samplerAnisotropy;
}

void VLDevice::PopulateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &createInfo) 
{
  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr;  // Optional
}

void VLDevice::SetupDebugMessenger() 
{
  if (!EnableValidationLayers) return;
  VkDebugUtilsMessengerCreateInfoEXT CreateInfo;
  PopulateDebugMessengerCreateInfo(CreateInfo);
  if (CreateDebugUtilsMessengerEXT(Instance, &CreateInfo, nullptr, &DebugMessenger) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}

bool VLDevice::CheckValidationLayerSupport() 
{
  uint32_t LayerCount;
  vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

  std::vector<VkLayerProperties> AvailableLayers(LayerCount);
  vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

  for (const char *LayerName : ValidationLayers) 
  {
    bool LayerFound = false;

    for (const auto &LayerProperties : AvailableLayers) 
    {
      if (strcmp(LayerName, LayerProperties.layerName) == 0) 
      {
        LayerFound = true;
        break;
      }
    }

    if (!LayerFound) 
    {
      return false;
    }
  }

  return true;
}

std::vector<const char *> VLDevice::GetRequiredExtensions() {
  uint32_t GlfwExtensionCount = 0;
  const char **GlfwExtensions;
  GlfwExtensions = glfwGetRequiredInstanceExtensions(&GlfwExtensionCount);

  std::vector<const char *> Extensions(GlfwExtensions, GlfwExtensions + GlfwExtensionCount);

  if (EnableValidationLayers) 
  {
    Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return Extensions;
}

void VLDevice::HasGflwRequiredInstanceExtensions() 
{
  uint32_t ExtensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, nullptr);
  std::vector<VkExtensionProperties> Extensions(ExtensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, Extensions.data());

  std::cout << "available extensions:" << std::endl;
  std::unordered_set<std::string> Available;
  for (const auto &Extension : Extensions) 
  {
    std::cout << "\t" << Extension.extensionName << std::endl;
    Available.insert(Extension.extensionName);
  }

  std::cout << "required extensions:" << std::endl;
  auto RequiredExtensions = GetRequiredExtensions();
  for (const auto &Required : RequiredExtensions) 
  {
    std::cout << "\t" << Required << std::endl;
    if (Available.find(Required) == Available.end()) 
    {
      throw std::runtime_error("Missing required GLFW extension");
    }
  }
}

bool VLDevice::CheckDeviceExtensionSupport(VkPhysicalDevice GetDevice) 
{
  uint32_t ExtensionCount;
  vkEnumerateDeviceExtensionProperties(GetDevice, nullptr, &ExtensionCount, nullptr);

  std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
  vkEnumerateDeviceExtensionProperties(
      GetDevice,
      nullptr,
      &ExtensionCount,
      AvailableExtensions.data());

  std::set<std::string> RequiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

  for (const auto &Extension : AvailableExtensions) 
  {
    RequiredExtensions.erase(Extension.extensionName);
  }

  return RequiredExtensions.empty();
}

QueueFamilyIndices VLDevice::FindQueueFamilies(VkPhysicalDevice GetDevice) 
{
  QueueFamilyIndices Indices;

  uint32_t QueueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(GetDevice, &QueueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(GetDevice, &QueueFamilyCount, QueueFamilies.data());

  int i = 0;
  for (const auto &QueueFamily : QueueFamilies) 
  {
    if (QueueFamily.queueCount > 0 && QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
    {
      Indices.GraphicsFamily = i;
      Indices.GraphicsFamilyHasValue = true;
    }
    VkBool32 PresentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(GetDevice, i, Surface, &PresentSupport);
    if (QueueFamily.queueCount > 0 && PresentSupport) 
    {
      Indices.PresentFamily = i;
      Indices.PresentFamilyHasValue = true;
    }
    if (Indices.IsComplete()) 
    {
      break;
    }

    i++;
  }

  return Indices;
}

SwapChainSupportDetails VLDevice::QuerySwapChainSupport(VkPhysicalDevice GetDevice) 
{
  SwapChainSupportDetails Details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GetDevice, Surface, &Details.Capabilities);

  uint32_t FormatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(GetDevice, Surface, &FormatCount, nullptr);

  if (FormatCount != 0) 
  {
    Details.Formats.resize(FormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(GetDevice, Surface, &FormatCount, Details.Formats.data());
  }

  uint32_t PresentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(GetDevice, Surface, &PresentModeCount, nullptr);

  if (PresentModeCount != 0) 
  {
    Details.PresentModes.resize(PresentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        GetDevice,
        Surface,
        &PresentModeCount,
        Details.PresentModes.data());
  }
  return Details;
}

VkFormat VLDevice::FindSupportedFormat(
    const std::vector<VkFormat> &Candidates, VkImageTiling Tiling, VkFormatFeatureFlags features) {
  for (VkFormat Format : Candidates) 
  {
    VkFormatProperties Props;
    vkGetPhysicalDeviceFormatProperties(PhysicalDevice, Format, &Props);

    if (Tiling == VK_IMAGE_TILING_LINEAR && (Props.linearTilingFeatures & features) == features) 
    {
      return Format;
    } else if (
        Tiling == VK_IMAGE_TILING_OPTIMAL && (Props.optimalTilingFeatures & features) == features) 
    {
      return Format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}

uint32_t VLDevice::FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags DeviceProperties) 
{
  VkPhysicalDeviceMemoryProperties MemProperties;
  vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemProperties);
  for (uint32_t i = 0; i < MemProperties.memoryTypeCount; i++) 
  {
    if ((TypeFilter & (1 << i)) &&
        (MemProperties.memoryTypes[i].propertyFlags & DeviceProperties) == DeviceProperties) 
    {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

void VLDevice::CreateBuffer(
    VkDeviceSize Size,
    VkBufferUsageFlags Usage,
    VkMemoryPropertyFlags DeviceProperties,
    VkBuffer &Buffer,
    VkDeviceMemory &BufferMemory) 
{
  VkBufferCreateInfo BufferInfo{};
  BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  BufferInfo.size = Size;
  BufferInfo.usage = Usage;
  BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(Device, &BufferInfo, nullptr, &Buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create vertex buffer!");
  }

  VkMemoryRequirements MemRequirements;
  vkGetBufferMemoryRequirements(Device, Buffer, &MemRequirements);

  VkMemoryAllocateInfo AllocInfo{};
  AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  AllocInfo.allocationSize = MemRequirements.size;
  AllocInfo.memoryTypeIndex = FindMemoryType(MemRequirements.memoryTypeBits, DeviceProperties);

  if (vkAllocateMemory(Device, &AllocInfo, nullptr, &BufferMemory) != VK_SUCCESS) 
  {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
  }

  vkBindBufferMemory(Device, Buffer, BufferMemory, 0);
}

VkCommandBuffer VLDevice::BeginSingleTimeCommands() 
{
  VkCommandBufferAllocateInfo AllocInfo{};
  AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  AllocInfo.commandPool = CommandPool;
  AllocInfo.commandBufferCount = 1;

  VkCommandBuffer CommandBuffer;
  vkAllocateCommandBuffers(Device, &AllocInfo, &CommandBuffer);

  VkCommandBufferBeginInfo BeginInfo{};
  BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(CommandBuffer, &BeginInfo);
  return CommandBuffer;
}

void VLDevice::EndSingleTimeCommands(VkCommandBuffer CommandBuffer) 
{
  vkEndCommandBuffer(CommandBuffer);

  VkSubmitInfo SubmitInfo{};
  SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  SubmitInfo.commandBufferCount = 1;
  SubmitInfo.pCommandBuffers = &CommandBuffer;

  vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(GraphicsQueue);

  vkFreeCommandBuffers(Device, CommandPool, 1, &CommandBuffer);
}

void VLDevice::CopyBuffer(VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize Size) 
{
  VkCommandBuffer CommandBuffer = BeginSingleTimeCommands();

  VkBufferCopy CopyRegion{};
  CopyRegion.srcOffset = 0;  // Optional
  CopyRegion.dstOffset = 0;  // Optional
  CopyRegion.size = Size;
  vkCmdCopyBuffer(CommandBuffer, SrcBuffer, DstBuffer, 1, &CopyRegion);

  EndSingleTimeCommands(CommandBuffer);
}

void VLDevice::CopyBufferToImage(
    VkBuffer Buffer, VkImage Image, uint32_t Width, uint32_t Height, uint32_t LayerCount) 
{
  VkCommandBuffer CommandBuffer = BeginSingleTimeCommands();

  VkBufferImageCopy Region{};
  Region.bufferOffset = 0;
  Region.bufferRowLength = 0;
  Region.bufferImageHeight = 0;

  Region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  Region.imageSubresource.mipLevel = 0;
  Region.imageSubresource.baseArrayLayer = 0;
  Region.imageSubresource.layerCount = LayerCount;

  Region.imageOffset = {0, 0, 0};
  Region.imageExtent = {Width, Height, 1};

  vkCmdCopyBufferToImage(
      CommandBuffer,
      Buffer,
      Image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &Region);
  EndSingleTimeCommands(CommandBuffer);
}

void VLDevice::CreateImageWithInfo(
    const VkImageCreateInfo &ImageInfo,
    VkMemoryPropertyFlags DeviceProperties,
    VkImage &Image,
    VkDeviceMemory &ImageMemory) 
{
  if (vkCreateImage(Device, &ImageInfo, nullptr, &Image) != VK_SUCCESS) 
  {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements MemRequirements;
  vkGetImageMemoryRequirements(Device, Image, &MemRequirements);

  VkMemoryAllocateInfo AllocInfo{};
  AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  AllocInfo.allocationSize = MemRequirements.size;
  AllocInfo.memoryTypeIndex = FindMemoryType(MemRequirements.memoryTypeBits, DeviceProperties);

  if (vkAllocateMemory(Device, &AllocInfo, nullptr, &ImageMemory) != VK_SUCCESS) 
  {
    throw std::runtime_error("failed to allocate image memory!");
  }

  if (vkBindImageMemory(Device, Image, ImageMemory, 0) != VK_SUCCESS) 
  {
    throw std::runtime_error("failed to bind image memory!");
  }
}

}  // namespace lve