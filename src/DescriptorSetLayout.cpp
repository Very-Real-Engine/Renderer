#include "DescriptorSetLayout.h"

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createGeometryPassDescriptorSetLayout() {
    std::unique_ptr<DescriptorSetLayout> descriptorSetLayout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
    descriptorSetLayout->initGeometryPassDescriptorSetLayout();
    return descriptorSetLayout;
}


void DescriptorSetLayout::cleanup() {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}


void DescriptorSetLayout::initGeometryPassDescriptorSetLayout() {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    // 디스크립터 레이아웃 설정
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;										// 바인딩 포인트 설정
    uboLayoutBinding.descriptorCount = 1;								// 디스크립터 개수 설정
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;	// 디스크립터 타입 설정
    uboLayoutBinding.pImmutableSamplers = nullptr;						// 이미지 샘플러 설정
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;			// 셰이더 타입 설정

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}


std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createLightingPassDescriptorSetLayout() {
    std::unique_ptr<DescriptorSetLayout> descriptorSetLayout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
    descriptorSetLayout->initLightingPassDescriptorSetLayout();
    return descriptorSetLayout;
}

void DescriptorSetLayout::initLightingPassDescriptorSetLayout() {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    VkDescriptorSetLayoutBinding inputAttachmentBinding0{};
    inputAttachmentBinding0.binding = 0;
    inputAttachmentBinding0.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    inputAttachmentBinding0.descriptorCount = 1;
    inputAttachmentBinding0.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    inputAttachmentBinding0.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding inputAttachmentBinding1{};
    inputAttachmentBinding1.binding = 1;
    inputAttachmentBinding1.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    inputAttachmentBinding1.descriptorCount = 1;
    inputAttachmentBinding1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    inputAttachmentBinding1.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding inputAttachmentBinding2{};
    inputAttachmentBinding2.binding = 2;
    inputAttachmentBinding2.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    inputAttachmentBinding2.descriptorCount = 1;
    inputAttachmentBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    inputAttachmentBinding2.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding lightingPassBufferBinding{};
    lightingPassBufferBinding.binding = 3;
    lightingPassBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightingPassBufferBinding.descriptorCount = 1;
    lightingPassBufferBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    lightingPassBufferBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {inputAttachmentBinding0, inputAttachmentBinding1, inputAttachmentBinding2, lightingPassBufferBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create LightingPass descriptor set layout!");
    }
}
