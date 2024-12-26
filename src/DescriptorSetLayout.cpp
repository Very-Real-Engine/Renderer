#include "DescriptorSetLayout.h"

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createDescriptorSetLayout() {
    std::unique_ptr<DescriptorSetLayout> descriptorSetLayout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
    descriptorSetLayout->initDescriptorSetLayout();
    return descriptorSetLayout;
}


void DescriptorSetLayout::cleanup() {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}


void DescriptorSetLayout::initDescriptorSetLayout() {
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


std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createGammaDescriptorSetLayout() {
    std::unique_ptr<DescriptorSetLayout> descriptorSetLayout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
    descriptorSetLayout->initGammaDescriptorSetLayout();
    return descriptorSetLayout;
}

void DescriptorSetLayout::initGammaDescriptorSetLayout() {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();


    VkDescriptorSetLayoutBinding gammaBufferBinding{};
    gammaBufferBinding.binding = 0;
    gammaBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    gammaBufferBinding.descriptorCount = 1;
    gammaBufferBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    gammaBufferBinding.pImmutableSamplers = nullptr;


    VkDescriptorSetLayoutBinding inputAttachmentBinding{};
    inputAttachmentBinding.binding = 1;
    inputAttachmentBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    inputAttachmentBinding.descriptorCount = 1;
    inputAttachmentBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    inputAttachmentBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {gammaBufferBinding, inputAttachmentBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create gamma descriptor set layout!");
    }
}