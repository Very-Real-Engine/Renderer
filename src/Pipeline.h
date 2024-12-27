#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "VulkanUtil.h"

class Pipeline {
public:
	static std::unique_ptr<Pipeline> createGeometryPassPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	static std::unique_ptr<Pipeline> createLightingPassPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	~Pipeline() {}
	void cleanup();


	VkPipeline getPipeline() { return pipeline; }
	VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

private:
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	void initGeometryPassPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	void initLightingPassPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);

	VkShaderModule createShaderModule(const std::vector<char>& code);
};
