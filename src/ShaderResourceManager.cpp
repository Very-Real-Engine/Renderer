#include "ShaderResourceManager.h"

std::unique_ptr<ShaderResourceManager> ShaderResourceManager::createGeometryPassShaderResourceManager(Scene* scene, VkDescriptorSetLayout descriptorSetLayout) {
    std::unique_ptr<ShaderResourceManager> shaderResourceManager = std::unique_ptr<ShaderResourceManager>(new ShaderResourceManager());
    shaderResourceManager->initGeometryPassShaderResourceManager(scene, descriptorSetLayout);
    return shaderResourceManager;
}


void ShaderResourceManager::cleanup() {
    for (size_t i = 0; i < m_uniformBuffers.size(); i++) {
        m_uniformBuffers[i]->cleanup();
    }
}


void ShaderResourceManager::initGeometryPassShaderResourceManager(Scene* scene, VkDescriptorSetLayout descriptorSetLayout) {
    createGeometryPassUniformBuffers(scene);
    createGeometryPassDescriptorSets(scene, descriptorSetLayout);
}


void ShaderResourceManager::createGeometryPassUniformBuffers(Scene* scene) {
    size_t objectCount = scene->getObjectCount();
    if (objectCount == 0) {
        throw std::runtime_error("failed to create uniform buffers!");
    }
    // 유니폼 버퍼에 저장 될 구조체의 크기
    VkDeviceSize bufferSize = sizeof(GeometryPassUniformBufferObject);

    // 각 요소들을 동시에 처리 가능한 최대 프레임 수만큼 만들어 둔다.
    m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT * objectCount);

    for (size_t i = 0; i < objectCount; i++) {
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
            m_uniformBuffers[i * MAX_FRAMES_IN_FLIGHT + j] = UniformBuffer::createUniformBuffer(bufferSize);
        }
    }
}


void ShaderResourceManager::createGeometryPassDescriptorSets(Scene* scene, VkDescriptorSetLayout descriptorSetLayout) {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();
    VkDescriptorPool descriptorPool = context.getDescriptorPool();

    size_t objectCount = scene->getObjectCount();
    std::vector<std::shared_ptr<Object>> objects = scene->getObjects();

    if (objectCount == 0) {
        throw std::runtime_error("failed to create descriptor sets!");
    }
    // 디스크립터 셋 레이아웃 벡터 생성 (기존 만들어놨던 디스크립터 셋 레이아웃 객체 이용)
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT * objectCount, descriptorSetLayout);

    // 디스크립터 셋 할당에 필요한 정보를 설정하는 구조체
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;										// 디스크립터 셋을 할당할 디스크립터 풀 지정
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * objectCount);		// 할당할 디스크립터 셋 개수 지정
    allocInfo.pSetLayouts = layouts.data();											// 할당할 디스크립터 셋 의 레이아웃을 정의하는 배열 

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT * objectCount);									// 디스크립터 셋을 저장할 벡터 크기 설정
    
    // 디스크립터 풀에 디스크립터 셋 할당
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }


    for (size_t i = 0; i < objectCount; i++) {
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
            // 디스크립터 셋에 바인딩할 버퍼 정보 
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_uniformBuffers[i * MAX_FRAMES_IN_FLIGHT + j]->getBuffer();			// 바인딩할 버퍼
            bufferInfo.offset = 0;												// 버퍼에서 데이터 시작 위치 offset
            bufferInfo.range = sizeof(GeometryPassUniformBufferObject);						// 셰이더가 접근할 버퍼 크기

            std::shared_ptr<Texture> texture = objects[i]->getTexture();
            VkDescriptorImageInfo imageInfo{};								
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 이미지의 레이아웃
            imageInfo.imageView = texture->getImageView();						// 셰이더에서 사용할 이미지 뷰
            imageInfo.sampler = texture->getSampler();							// 이미지 샘플링에 사용할 샘플러 설정
            
            // 디스크립터 셋 바인딩 및 업데이트
            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i * MAX_FRAMES_IN_FLIGHT + j];					// 업데이트 할 디스크립터 셋
            descriptorWrites[0].dstBinding = 0;													// 업데이트 할 바인딩 포인트
            descriptorWrites[0].dstArrayElement = 0;											// 업데이트 할 디스크립터가 배열 타입인 경우 해당 배열의 원하는 index 부터 업데이트 가능 (배열 아니면 0으로 지정)
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;				// 업데이트 할 디스크립터 타입
            descriptorWrites[0].descriptorCount = 1;											// 업데이트 할 디스크립터 개수
            descriptorWrites[0].pBufferInfo = &bufferInfo;										// 업데이트 할 버퍼 디스크립터 정보 구조체 배열

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i * MAX_FRAMES_IN_FLIGHT + j];					// 업데이트 할 디스크립터 셋
            descriptorWrites[1].dstBinding = 1;													// 업데이트 할 바인딩 포인트
            descriptorWrites[1].dstArrayElement = 0;											// 업데이트 할 디스크립터가 배열 타입인 경우 해당 배열의 원하는 index 부터 업데이트 가능 (배열 아니면 0으로 지정)
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;		// 업데이트 할 디스크립터 타입
            descriptorWrites[1].descriptorCount = 1;											// 업데이트 할 디스크립터 개수
            descriptorWrites[1].pImageInfo = &imageInfo;										// 업데이트 할 버퍼 디스크립터 정보 구조체 배열

            // 디스크립터 셋을 업데이트 하여 사용할 리소스 바인딩
            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }
}


std::unique_ptr<ShaderResourceManager> ShaderResourceManager::createLightingPassShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout, 
    VkImageView positionImageView, VkImageView normalImageView, VkImageView albedoImageView) {
    std::unique_ptr<ShaderResourceManager> shaderResourceManager = std::unique_ptr<ShaderResourceManager>(new ShaderResourceManager());
    shaderResourceManager->initLightingPassShaderResourceManager(descriptorSetLayout, positionImageView, normalImageView, albedoImageView);
    return shaderResourceManager;
}


void ShaderResourceManager::initLightingPassShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout, 
    VkImageView positionImageView, VkImageView normalImageView, VkImageView albedoImageView) {
    createLightingPassUniformBuffers();
    createLightingPassDescriptorSets(descriptorSetLayout, positionImageView, normalImageView, albedoImageView);
}

void ShaderResourceManager::createLightingPassUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(LightingPassUniformBufferObject);

    m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_uniformBuffers[i] = UniformBuffer::createUniformBuffer(bufferSize);
    }
}

void ShaderResourceManager::createLightingPassDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, 
    VkImageView positionImageView, VkImageView normalImageView, VkImageView albedoImageView) {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();
    VkDescriptorPool descriptorPool = context.getDescriptorPool();

    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

        VkDescriptorImageInfo positionImageInfo{};
        positionImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        positionImageInfo.imageView = positionImageView;
        positionImageInfo.sampler = VK_NULL_HANDLE;

        VkDescriptorImageInfo normalImageInfo{};
        normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normalImageInfo.imageView = normalImageView;
        normalImageInfo.sampler = VK_NULL_HANDLE;

        VkDescriptorImageInfo albedoImageInfo{};
        albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        albedoImageInfo.imageView = albedoImageView;
        albedoImageInfo.sampler = VK_NULL_HANDLE;

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i]->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(LightingPassUniformBufferObject);

        std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &positionImageInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &normalImageInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &albedoImageInfo;

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}
