#pragma once

#include "VLDevice.h"

#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace VulkanLearn {

    class VLSwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        VLSwapChain(VLDevice& DeviceRef, VkExtent2D windowExtend);
        VLSwapChain(VLDevice& DeviceRef, VkExtent2D WindowExtent, std::shared_ptr<VLSwapChain> PreviousSwapChain);
        ~VLSwapChain();

        VLSwapChain(const VLSwapChain&) = delete;
        VLSwapChain(VLSwapChain&&) = delete;
        VLSwapChain& operator=(const VLSwapChain&) = delete;
        VLSwapChain& operator=(VLSwapChain&&) = delete;

        VkFramebuffer GetFrameBuffer(int index) { return SwapChainFramebuffers[index]; }
        VkRenderPass GetRenderPass() { return RenderPass; }
        VkImageView GetImageView(int index) { return SwapChainImageViews[index]; }
        size_t GetImageCount() { return SwapChainImages.size(); }
        VkFormat GetSwapChainImageFormat() { return SwapChainImageFormat; }
        VkExtent2D GetSwapChainExtent() { return SwapChainExtent; }
        uint32_t GetWidth() { return SwapChainExtent.width; }
        uint32_t GetHeight() { return SwapChainExtent.height; }

        float GetExtentAspectRatio() {
            return static_cast<float>(SwapChainExtent.width) / static_cast<float>(SwapChainExtent.height);
        }
        VkFormat FindDepthFormat();

        VkResult AcquireNextImage(uint32_t* ImageIndex);
        VkResult SubmitCommandBuffers(const VkCommandBuffer* Buffers, uint32_t* imageIndex);

    private:
        void Init();
        void CreateSwapChain();
        void CreateImageViews();
        void CreateDepthResources();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& AvailableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& AvailablePresentModes);
        // Gives us the resolution of the swap chain images (most of the time = window resolution)
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities);

        VkFormat SwapChainImageFormat;
        VkExtent2D SwapChainExtent;

        std::vector<VkFramebuffer> SwapChainFramebuffers;
        VkRenderPass RenderPass;

        std::vector<VkImage> DepthImages;
        std::vector<VkDeviceMemory> DepthImageMemorys;
        std::vector<VkImageView> DepthImageViews;
        std::vector<VkImage> SwapChainImages;
        std::vector<VkImageView> SwapChainImageViews;

        // Note:	Memory unsafe, device can be released before the pipeline (Aggregation)
        //		Only use implicitly that our member variable will outlive any 
        //		instances of the containing class that depend on it
        //		This is not a problem as a pipeline fundamentally needs a device to exist
        VLDevice& Device;
        VkExtent2D WindowExtent;

        VkSwapchainKHR SwapChain;
        std::shared_ptr<VLSwapChain> OldSwapChain;

        std::vector<VkSemaphore> ImageAvailableSemaphores;
        std::vector<VkSemaphore> RenderFinishedSemaphores;
        std::vector<VkFence> InFlightFences;
        std::vector<VkFence> ImagesInFlight;
        size_t CurrentFrame = 0;
    };

}  // namespace VulkanLearn
