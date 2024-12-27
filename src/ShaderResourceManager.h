#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "Buffer.h"
#include "Scene.h"

class ShaderResourceManager {
public:
	static std::unique_ptr<ShaderResourceManager> createGeometryPassShaderResourceManager(Scene* scene, VkDescriptorSetLayout descriptorSetLayout);
	static std::unique_ptr<ShaderResourceManager> createLightingPassShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout, VkImageView resolveImageView);
	~ShaderResourceManager() {}
	void cleanup();

	std::vector<std::shared_ptr<UniformBuffer>>& getUniformBuffers() { return m_uniformBuffers; }
	std::vector<VkDescriptorSet>& getDescriptorSets() { return descriptorSets; }

private:
	std::vector<std::shared_ptr<UniformBuffer>> m_uniformBuffers;
	std::vector<VkDescriptorSet> descriptorSets;

	void initGeometryPassShaderResourceManager(Scene* scene, VkDescriptorSetLayout descriptorSetLayout);
	void createGeometryPassUniformBuffers(Scene* scene);
	void createGeometryPassDescriptorSets(Scene* scene, VkDescriptorSetLayout descriptorSetLayout);


	void initLightingPassShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout, VkImageView resolveImageView);
	void createLightingPassUniformBuffers();
	void createLightingPassDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkImageView resolveImageView);
};
