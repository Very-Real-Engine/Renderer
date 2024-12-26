#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "VulkanUtil.h"

class RenderPass {
public:
	static std::unique_ptr<RenderPass> createRenderPass(VkFormat swapChainImageFormat);
	~RenderPass() {}
	void cleanup();

	static std::unique_ptr<RenderPass> createGammaRenderPass(VkFormat swapChainImageFormat);

	VkRenderPass getRenderPass() { return renderPass; }

private:
	VkRenderPass renderPass;

	void initRenderPass(VkFormat swapChainImageFormat);
	void initGammaRenderPass(VkFormat swapChainImageFormat);
};
