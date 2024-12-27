#pragma once

#include "Common.h"
#include "VulkanContext.h"

class DescriptorSetLayout {
public:
	static std::unique_ptr<DescriptorSetLayout> createGeometryPassDescriptorSetLayout();
	static std::unique_ptr<DescriptorSetLayout> createLightingPassDescriptorSetLayout();
	~DescriptorSetLayout() {}
	void cleanup();

	VkDescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; }

private:
	VkDescriptorSetLayout descriptorSetLayout;

	void initGeometryPassDescriptorSetLayout();
	void initLightingPassDescriptorSetLayout();
};
