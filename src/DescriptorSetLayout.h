#pragma once

#include "Common.h"
#include "VulkanContext.h"

class DescriptorSetLayout {
public:
	static std::unique_ptr<DescriptorSetLayout> createDescriptorSetLayout();
	~DescriptorSetLayout() {}
	void cleanup();

	// gamma
	static std::unique_ptr<DescriptorSetLayout> createGammaDescriptorSetLayout();

	VkDescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; }

private:
	VkDescriptorSetLayout descriptorSetLayout;

	void initDescriptorSetLayout();

	// gamma
	void initGammaDescriptorSetLayout();
};
