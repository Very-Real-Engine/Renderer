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
	VkImageView& getDepthImageView() { return depthImageView; }
	VkImageView& getPositionImageView() { return positionImageView; }
	VkImageView& getNormalImageView() { return normalImageView; }
	VkImageView& getAlbedoImageView() { return albedoImageView; }

private:
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	
	VkImage positionImage;
	VkDeviceMemory positionImageMemory;
	VkImageView positionImageView;

	VkImage normalImage;
	VkDeviceMemory normalImageMemory;
	VkImageView normalImageView;

	VkImage albedoImage;
	VkDeviceMemory albedoImageMemory;
	VkImageView albedoImageView;

	std::vector<VkFramebuffer> framebuffers;
};
