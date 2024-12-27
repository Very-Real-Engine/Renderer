#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "VulkanUtil.h"

class RenderPass {
public:
	static std::unique_ptr<RenderPass> createRenderPass(VkFormat swapChainImageFormat);
	static std::unique_ptr<RenderPass> createDeferredRenderPass(VkFormat swapChainImageFormat);
	~RenderPass() {}
	void cleanup();


	VkRenderPass getRenderPass() { return renderPass; }

private:
	VkRenderPass renderPass;

	void initRenderPass(VkFormat swapChainImageFormat);
	void initDeferredRenderPass(VkFormat swapChainImageFormat);
};
