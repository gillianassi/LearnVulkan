#include "VLSwapChain.h"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace VulkanLearn 
{

	VLSwapChain::VLSwapChain(VLDevice& deviceRef, VkExtent2D extent)
		: Device{ deviceRef }, WindowExtent{ extent } 
	{
		CreateSwapChain();
		// This describes how to access the image and which part of the image to access
		CreateImageViews();
		// This describes the structure and format of our frame buffer objects and their attachments
		CreateRenderPass();
		CreateDepthResources();
		CreateFramebuffers();
		CreateSyncObjects();
	}

	VLSwapChain::~VLSwapChain() 
	{
		for (auto imageView : SwapChainImageViews) 
		{
			vkDestroyImageView(Device.GetDevice(), imageView, nullptr);
		}
		SwapChainImageViews.clear();

		if (SwapChain != nullptr) 
		{
			vkDestroySwapchainKHR(Device.GetDevice(), SwapChain, nullptr);
			SwapChain = nullptr;
		}

		for (int i = 0; i < DepthImages.size(); i++) 
		{
			vkDestroyImageView(Device.GetDevice(), DepthImageViews[i], nullptr);
			vkDestroyImage(Device.GetDevice(), DepthImages[i], nullptr);
			vkFreeMemory(Device.GetDevice(), DepthImageMemorys[i], nullptr);
		}

		for (auto framebuffer : SwapChainFramebuffers) 
		{
			vkDestroyFramebuffer(Device.GetDevice(), framebuffer, nullptr);
		}

		vkDestroyRenderPass(Device.GetDevice(), RenderPass, nullptr);

		// cleanup synchronization objects
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
		{
			vkDestroySemaphore(Device.GetDevice(), RenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(Device.GetDevice(), ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(Device.GetDevice(), InFlightFences[i], nullptr);
		}
	}

	VkResult VLSwapChain::AcquireNextImage(uint32_t* ImageIndex) 
	{
		vkWaitForFences(
			Device.GetDevice(),
			1,
			&InFlightFences[CurrentFrame],
			VK_TRUE,
			std::numeric_limits<uint64_t>::max());

		VkResult result = vkAcquireNextImageKHR(
			Device.GetDevice(),
			SwapChain,
			std::numeric_limits<uint64_t>::max(),
			ImageAvailableSemaphores[CurrentFrame],  // must be a not signaled semaphore
			VK_NULL_HANDLE,
			ImageIndex);

		return result;
	}

	VkResult VLSwapChain::SubmitCommandBuffers(
		const VkCommandBuffer* Buffers, uint32_t* ImageIndex) 
	{
		if (ImagesInFlight[*ImageIndex] != VK_NULL_HANDLE) 
		{
			vkWaitForFences(Device.GetDevice(), 1, &ImagesInFlight[*ImageIndex], VK_TRUE, UINT64_MAX);
		}
		ImagesInFlight[*ImageIndex] = InFlightFences[CurrentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { ImageAvailableSemaphores[CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = Buffers;

		VkSemaphore signalSemaphores[] = { RenderFinishedSemaphores[CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(Device.GetDevice(), 1, &InFlightFences[CurrentFrame]);
		if (vkQueueSubmit(Device.GetGraphicsQueue(), 1, &submitInfo, InFlightFences[CurrentFrame]) !=
			VK_SUCCESS) 
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = ImageIndex;

		auto result = vkQueuePresentKHR(Device.GetPresentQueue(), &presentInfo);

		CurrentFrame = (CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		return result;
	}

	void VLSwapChain::CreateSwapChain() 
	{
		SwapChainSupportDetails swapChainSupport = Device.GetSwapChainSupport();

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentationModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.Capabilities);

		// Note:	It is recommended to request at least one more image than the minimum, as sticking to this minimum
		//			would mean that we have to wait on the driver to complete internal operations before acquiring 
		//			another image to render to. But don't go over the max.
		uint32_t GetImageCount = swapChainSupport.Capabilities.minImageCount + 1;
		if (swapChainSupport.Capabilities.maxImageCount > 0 &&
			GetImageCount > swapChainSupport.Capabilities.maxImageCount) 
		{
			GetImageCount = swapChainSupport.Capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = Device.GetSurface();

		createInfo.minImageCount = GetImageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		// Would only be >1 for stereoscopic 3d applications
		createInfo.imageArrayLayers = 1;
		// Note:	Specifies for what the images in the swap chain will get used
		//			Rendering directly to them is color attachment. For rendering to a separate image for PP use 
		//			VK_IMAGE_USAGE_TRANSFER_DST_BIT with a memory operation to transfer the rendered image to a swap 
		//			chain image
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// Specify how to handle swap chain images that are used across multiple queue families
		// (This is when graphics queue != persentation queue)
		QueueFamilyIndices indices = Device.FindPhysicalQueueFamilies();
		uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentationFamily.value() };

		// When the families differ, use concurrent mode to avoid ownership management for now
		// TODO: Always use exclusive mode and handle ownership
		if (indices.GraphicsFamily != indices.PresentationFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;      // Optional
			createInfo.pQueueFamilyIndices = nullptr;  // Optional
		}

		// Ignore the alpha channel
		createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;
		// we don't care about the color of pixels that are obscured
		createInfo.clipped = VK_TRUE;

		// Note:	Important for when the swap chain becomes invalid, will handle this in the future
		//			For now, assume only one swap chain will be created
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(Device.GetDevice(), &createInfo, nullptr, &SwapChain) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create swap chain!");
		}

		// we only specified a minimum number of images in the swap chain, so the implementation is
		// allowed to create a swap chain with more. That's why we'll first query the final number of
		// images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
		// retrieve the handles.
		vkGetSwapchainImagesKHR(Device.GetDevice(), SwapChain, &GetImageCount, nullptr);
		SwapChainImages.resize(GetImageCount);
		vkGetSwapchainImagesKHR(Device.GetDevice(), SwapChain, &GetImageCount, SwapChainImages.data());

		SwapChainImageFormat = surfaceFormat.format;
		SwapChainExtent = extent;
	}

	void VLSwapChain::CreateImageViews() {
		SwapChainImageViews.resize(SwapChainImages.size());
		for (size_t i = 0; i < SwapChainImages.size(); i++) 
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = SwapChainImages[i];
			// specify how the image needs to be treated
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = SwapChainImageFormat;
			// Lets you swizzle color channels
			viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			// Describe the image's purpose
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			// stereographic 3D applications can use different layers to set a different view for each eye
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(Device.GetDevice(), &viewInfo, nullptr, &SwapChainImageViews[i]) !=
				VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}
		}
	}

	// ToDo: Move this out of the SwapChain and create something specific for render passes
	void VLSwapChain::CreateRenderPass() 
	{
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = FindDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = GetSwapChainImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstSubpass = 0;
		dependency.dstStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask =
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(Device.GetDevice(), &renderPassInfo, nullptr, &RenderPass) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void VLSwapChain::CreateFramebuffers() {
		SwapChainFramebuffers.resize(GetImageCount());
		for (size_t i = 0; i < GetImageCount(); i++) {
			std::array<VkImageView, 2> attachments = { SwapChainImageViews[i], DepthImageViews[i] };

			VkExtent2D SwapChainExtent = GetSwapChainExtent();
			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = RenderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = SwapChainExtent.width;
			framebufferInfo.height = SwapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(
				Device.GetDevice(),
				&framebufferInfo,
				nullptr,
				&SwapChainFramebuffers[i]) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void VLSwapChain::CreateDepthResources() 
	{
		VkFormat depthFormat = FindDepthFormat();
		VkExtent2D SwapChainExtent = GetSwapChainExtent();

		DepthImages.resize(GetImageCount());
		DepthImageMemorys.resize(GetImageCount());
		DepthImageViews.resize(GetImageCount());

		for (int i = 0; i < DepthImages.size(); i++) 
		{
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = SwapChainExtent.width;
			imageInfo.extent.height = SwapChainExtent.height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = depthFormat;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.flags = 0;

			Device.CreateImageWithInfo(
				imageInfo,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				DepthImages[i],
				DepthImageMemorys[i]);

			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = DepthImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = depthFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(Device.GetDevice(), &viewInfo, nullptr, &DepthImageViews[i]) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to create texture image view!");
			}
		}
	}

	void VLSwapChain::CreateSyncObjects()
	{
		ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		ImagesInFlight.resize(GetImageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
		{
			if (vkCreateSemaphore(Device.GetDevice(), &semaphoreInfo, nullptr, &ImageAvailableSemaphores[i]) !=
				VK_SUCCESS ||
				vkCreateSemaphore(Device.GetDevice(), &semaphoreInfo, nullptr, &RenderFinishedSemaphores[i]) !=
				VK_SUCCESS ||
				vkCreateFence(Device.GetDevice(), &fenceInfo, nullptr, &InFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	VkSurfaceFormatKHR VLSwapChain::ChooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>& AvailableFormats)
	{
		for (const auto& availableFormat : AvailableFormats) 
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
			{
				return availableFormat;
			}
		}
		return AvailableFormats[0];
	}

	VkPresentModeKHR VLSwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes)
	{
		// Note:    Mailbox lowers latency but GPU Never idles. If no additional manner of throttling is implemented,
		//          this consumes a lot of power -> not ideal for mobile
		for (const auto& availablePresentMode : AvailablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				std::cout << "Present mode: Mailbox" << std::endl;
				return availablePresentMode;
			}
		}

		// Note:	Immediate present mode doesn't perform any synchronization with the refresh cycle of the display
		//			It submits the images right away to the screen
		//			when updating the current image which might result in tearing
		//			Also uses a lot of power, so not ideal for mobile
		// for (const auto &availablePresentMode : availablePresentModes) {
		//   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
		//     std::cout << "Present mode: Immediate" << std::endl;
		//     return availablePresentMode;
		//   }
		// }

		// Note:	Uses FIFO, after back buffers have been written to lets GPU idle  until the next v-sync cycle
		//			This causes bad latency but is better for mobile
		std::cout << "Present mode: V-Sync" << std::endl;
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VLSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities) 
	{
		if (Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
		{
			return Capabilities.currentExtent;
		}
		else 
		{
			VkExtent2D actualExtent = WindowExtent;
			actualExtent.width = std::max(
				Capabilities.minImageExtent.width,
				std::min(Capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(
				Capabilities.minImageExtent.height,
				std::min(Capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	VkFormat VLSwapChain::FindDepthFormat() 
	{
		return Device.FindSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

}  // namespace lve
