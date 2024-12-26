#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "SwapChain.h"

class FrameBuffers {
public:
	static std::unique_ptr<FrameBuffers> createSwapChainFrameBuffers(SwapChain* swapChain, VkRenderPass renderPass);
	~FrameBuffers() {}
	void cleanup();

	void initSwapChainFrameBuffers(SwapChain* swapChain, VkRenderPass renderPass);

	std::vector<VkFramebuffer>& getFramebuffers() { return framebuffers; }
	VkImageView& getColorImageView() { return colorImageView; }
	VkImageView& getDepthImageView() { return depthImageView; }
	VkImageView& getResolveImageView() { return resolveImageView; }

private:
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	std::vector<VkFramebuffer> framebuffers;

	VkImage resolveImage;
	VkDeviceMemory resolveImageMemory;
	VkImageView resolveImageView;
};
