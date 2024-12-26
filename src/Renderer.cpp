#include "Renderer.h"

std::unique_ptr<Renderer> Renderer::createRenderer(GLFWwindow* window) {
    std::unique_ptr<Renderer> renderer = std::unique_ptr<Renderer>(new Renderer());
    renderer->init(window);
    return renderer;
}


void Renderer::init(GLFWwindow* window) {
    this->window = window;

    auto &context = VulkanContext::getContext();
    context.initContext(window);
    surface = context.getSurface();
    physicalDevice = context.getPhysicalDevice();
    device = context.getDevice();
    graphicsQueue = context.getGraphicsQueue();
    presentQueue = context.getPresentQueue();
    commandPool = context.getCommandPool();
    msaaSamples = context.getMsaaSamples();
    descriptorPool = context.getDescriptorPool();

    m_swapChain = SwapChain::createSwapChain(window);
    swapChain = m_swapChain->getSwapChain();
    swapChainImages = m_swapChain->getSwapChainImages();
    swapChainImageFormat = m_swapChain->getSwapChainImageFormat();
    swapChainExtent = m_swapChain->getSwapChainExtent();
    swapChainImageViews = m_swapChain->getSwapChainImageViews();


    m_syncObjects = SyncObjects::createSyncObjects();
    imageAvailableSemaphores = m_syncObjects->getImageAvailableSemaphores();
    renderFinishedSemaphores = m_syncObjects->getRenderFinishedSemaphores();
    inFlightFences = m_syncObjects->getInFlightFences();

    // m_renderPass = RenderPass::createRenderPass(swapChainImageFormat);
    // renderPass = m_renderPass->getRenderPass();

    m_gammaRenderPass = RenderPass::createGammaRenderPass(swapChainImageFormat);
    gammaRenderPass = m_gammaRenderPass->getRenderPass();

    m_swapChainFrameBuffers = FrameBuffers::createSwapChainFrameBuffers(m_swapChain.get(), gammaRenderPass);
    swapChainFramebuffers = m_swapChainFrameBuffers->getFramebuffers();

    m_descriptorSetLayout = DescriptorSetLayout::createDescriptorSetLayout();
    descriptorSetLayout = m_descriptorSetLayout->getDescriptorSetLayout();

    // gamma
    m_gammaDescriptorSetLayout = DescriptorSetLayout::createGammaDescriptorSetLayout();
    gammaDescriptorSetLayout = m_gammaDescriptorSetLayout->getDescriptorSetLayout();

    m_gammaShaderResourceManager = ShaderResourceManager::createGammaShaderResourceManager(gammaDescriptorSetLayout, m_swapChainFrameBuffers->getResolveImageView());
    gammaDescriptorSets = m_gammaShaderResourceManager->getDescriptorSets();

    // m_pipeline = Pipeline::createPipeline(renderPass, descriptorSetLayout);
    m_pipeline = Pipeline::createPipeline(gammaRenderPass, descriptorSetLayout);
    pipelineLayout = m_pipeline->getPipelineLayout();
    graphicsPipeline = m_pipeline->getPipeline();

    m_gammaPipeline = Pipeline::createGammaPipeline(gammaRenderPass, gammaDescriptorSetLayout);
    gammaPipelineLayout = m_gammaPipeline->getPipelineLayout();
    gammaGraphicsPipeline = m_gammaPipeline->getPipeline();

    m_commandBuffers = CommandBuffers::createCommandBuffers();
    commandBuffers = m_commandBuffers->getCommandBuffers();

}


void Renderer::cleanup() {
    m_swapChainFrameBuffers->cleanup();
    m_swapChain->cleanup();

    m_pipeline->cleanup();
    m_gammaPipeline->cleanup();

    // m_renderPass->cleanup();
    m_gammaRenderPass->cleanup();

    m_shaderResourceManager->cleanup();
    m_gammaShaderResourceManager->cleanup();

    m_descriptorSetLayout->cleanup();
    m_gammaDescriptorSetLayout->cleanup();

    m_syncObjects->cleanup();
    VulkanContext::getContext().cleanup();
}


void Renderer::loadScene(Scene* scene) {
    m_shaderResourceManager = ShaderResourceManager::createShaderResourceManager(scene, m_descriptorSetLayout->getDescriptorSetLayout());
    descriptorSets = m_shaderResourceManager->getDescriptorSets();
    m_uniformBuffers = m_shaderResourceManager->getUniformBuffers();
}


void Renderer::drawFrame(Scene* scene) {
    // [이전 GPU 작업 대기]
    // 동시에 작업 가능한 최대 Frame 개수만큼 작업 중인 경우 대기 (가장 먼저 시작한 Frame 작업이 끝나서 Fence에 signal을 보내기를 기다림)
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // [작업할 image 준비]
    // 이번 Frame 에서 사용할 이미지 준비 및 해당 이미지 index 받아오기 (준비가 끝나면 signal 보낼 세마포어 등록)
    // vkAcquireNextImageKHR 함수는 CPU에서 swapChain과 surface의 호환성을 확인하고 GPU에 이미지 준비 명령을 내리는 함수
    // 만약 image가 프레젠테이션 큐에 작업이 진행 중이거나 대기 중이면 해당 image는 사용하지 않고 대기한다.
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    // image 준비 실패로 인한 오류 처리
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // 스왑 체인이 surface 크기와 호환되지 않는 경우로(창 크기 변경), 스왑 체인 재생성 후 다시 draw
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        // 진짜 오류 gg
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // [Fence 초기화]
    // Fence signal 상태 not signaled 로 초기화
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    // [Command Buffer에 명령 기록]
    // 커맨드 버퍼 초기화 및 명령 기록
    vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0); // 두 번째 매개변수인 Flag 를 0으로 초기화하면 기본 초기화 진행
    // recordCommandBuffer(scene, commandBuffers[currentFrame], imageIndex); // 현재 작업할 image의 index와 commandBuffer를 전송
    recordGammaCommandBuffer(scene, commandBuffers[currentFrame], imageIndex); // 현재 작업할 image의 index와 commandBuffer를 전송

    // [렌더링 Command Buffer 제출]
    // 렌더링 커맨드 버퍼 제출 정보 객체 생성
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // 작업 실행 신호를 받을 대기 세마포어 설정 (해당 세마포어가 signal 상태가 되기 전엔 대기)
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};				
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; 	
    submitInfo.waitSemaphoreCount = 1;														// 대기 세마포어 개수
    submitInfo.pWaitSemaphores = waitSemaphores;											// 대기 세마포어 등록
    submitInfo.pWaitDstStageMask = waitStages;												// 대기할 시점 등록 (그 전까지는 세마포어 상관없이 그냥 진행)	

    // 커맨드 버퍼 등록
    submitInfo.commandBufferCount = 1;														// 커맨드 버퍼 개수 등록
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];								// 커매드 버퍼 등록

    // 작업이 완료된 후 신호를 보낼 세마포어 설정 (작업이 끝나면 해당 세마포어 signal 상태로 변경)
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;													// 작업 끝나고 신호를 보낼 세마포어 개수
    submitInfo.pSignalSemaphores = signalSemaphores;										// 작업 끝나고 신호를 보낼 세마포어 등록

    // 커맨드 버퍼 제출 
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // [프레젠테이션 Command Buffer 제출]
    // 프레젠테이션 커맨드 버퍼 제출 정보 객체 생성
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    // 작업 실행 신호를 받을 대기 세마포어 설정
    presentInfo.waitSemaphoreCount = 1;														// 대기 세마포어 개수
    presentInfo.pWaitSemaphores = signalSemaphores;											// 대기 세마포어 등록

    // 제출할 스왑 체인 설정
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;															// 스왑체인 개수
    presentInfo.pSwapchains = swapChains;													// 스왑체인 등록
    presentInfo.pImageIndices = &imageIndex;												// 스왑체인에서 표시할 이미지 핸들 등록

    // 프레젠테이션 큐에 이미지 제출
    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    // 프레젠테이션 실패 오류 발생 시
    // if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) { <- framebufferResized는 명시적으로 해줄뿐 사실상 필요하지가 않음 나중에 수정할꺼면 하자
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // 스왑 체인 크기와 surface의 크기가 호환되지 않는 경우
        recreateSwapChain(); 	// 변경된 surface에 맞는 SwapChain, ImageView, FrameBuffer 생성 
    } else if (result != VK_SUCCESS) {
        // 진짜 오류 gg
        throw std::runtime_error("failed to present swap chain image!");
    }
    // [프레임 인덱스 증가]
    // 다음 작업할 프레임 변경
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}


/*
    [커맨드 버퍼에 작업 기록]
    1. 커맨드 버퍼 기록 시작
    2. 렌더패스 시작하는 명령 기록
    3. 파이프라인 설정 명령 기록
    4. 렌더링 명령 기록
    5. 렌더 패스 종료 명령 기록
    6. 커맨드 버퍼 기록 종료
*/
void Renderer::recordCommandBuffer(Scene* scene, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    
    // 커맨드 버퍼 기록을 위한 정보 객체
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // GPU에 필요한 작업을 모두 커맨드 버퍼에 기록하기 시작
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // 렌더 패스 정보 지정
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;								// 렌더 패스 등록
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];		// 프레임 버퍼 등록
    renderPassInfo.renderArea.offset = {0, 0};							// 렌더링 시작 좌표 등록
    renderPassInfo.renderArea.extent = swapChainExtent;					// 렌더링 width, height 등록 (보통 프레임버퍼, 스왑체인의 크기와 같게 설정)

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};				

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());		// clear color 개수 등록
    renderPassInfo.pClearValues = clearValues.data();								// clear color 등록 (첨부한 attachment 개수와 같게 등록)
    
    /* 
        [렌더 패스를 시작하는 명령을 기록] 
        GPU에서 렌더링에 필요한 자원과 설정을 준비 (대략 과정)
        1. 렌더링 자원 초기화 (프레임 버퍼와 렌더 패스에 등록된 attachment layout 초기화)
        2. 서브패스 및 attachment 설정 적용
        3. 렌더링 작업을 위한 컨텍스트 준비 (뷰포트, 시저 등 설정)
    */
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    //	[사용할 그래픽 파이프 라인을 설정하는 명령 기록]
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    // 뷰포트 정보 입력
    VkViewport viewport{};
    viewport.x = 0.0f;									// 뷰포트의 시작 x 좌표
    viewport.y = 0.0f;									// 뷰포트의 시작 y 좌표
    viewport.width = (float) swapChainExtent.width;		// 뷰포트의 width 크기
    viewport.height = (float) swapChainExtent.height;	// 뷰포트의 height 크기
    viewport.minDepth = 0.0f;							// 뷰포트의 최소 깊이
    viewport.maxDepth = 1.0f;							// 뷰포트의 최대 깊이
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);	// [커맨드 버퍼에 뷰포트 설정 등록]

    // 시저 정보 입력
    VkRect2D scissor{};
    scissor.offset = {0, 0};							// 시저의 시작 좌표
    scissor.extent = swapChainExtent;					// 시저의 width, height
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);		// [커맨드 버퍼에 시저 설정 등록]
    

    // [렌더링 명령 기록]
    const std::vector<std::shared_ptr<Object>>& objects = scene->getObjects();
    size_t objectCount = scene->getObjectCount();

    for (size_t i = 0; i < objectCount; i++) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[MAX_FRAMES_IN_FLIGHT * i + currentFrame], 0, nullptr);
        UniformBufferObject ubo{};
        ubo.model = objects[i]->getModelMatrix();
        ubo.view = scene->getViewMatrix();
        ubo.proj = scene->getProjMatrix(swapChainExtent);
        ubo.proj[1][1] *= -1;
        // memcpy(uniformBuffersMapped[currentFrame * objectCount + i], &ubo, sizeof(ubo));
        m_uniformBuffers[MAX_FRAMES_IN_FLIGHT * i + currentFrame]->updateUniformBuffer(&ubo, sizeof(ubo));
        objects[i]->draw(commandBuffer);
    }
    /*
        [렌더 패스 종료]
        1. 자원의 정리 및 레이아웃 전환 (최종 작업을 위해 attachment에 정의된 finalLayout 설정)
        2. Load, Store 작업 (각 attachment에 정해진 load, store 작업 실행)
        3. 렌더 패스의 종료를 GPU에 알려 자원 재활용 등이 가능해짐
    */ 
    vkCmdEndRenderPass(commandBuffer);

    // [커맨드 버퍼 기록 종료]
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}


/*
    변경된 window 크기에 맞게 SwapChain, ImageView, FrameBuffer 재생성
*/
void Renderer::recreateSwapChain() {
    // 현재 프레임버퍼 사이즈 체크
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    
    // 현재 프레임 버퍼 사이즈가 0이면 다음 이벤트 호출까지 대기
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents(); // 다음 이벤트 발생 전까지 대기하여 CPU 사용률을 줄이는 함수 
    }

    // 모든 GPU 작업 종료될 때까지 대기 (사용중인 리소스를 건들지 않기 위해)
    vkDeviceWaitIdle(device);

    // 스왑 체인 관련 리소스 정리
    m_swapChainFrameBuffers->cleanup();
    m_swapChain->recreateSwapChain();

    // 현재 window 크기에 맞게 SwapChain, DepthResource, ImageView, FrameBuffer 재생성
    swapChain = m_swapChain->getSwapChain();
    swapChainImages = m_swapChain->getSwapChainImages();
    swapChainImageFormat = m_swapChain->getSwapChainImageFormat();
    swapChainExtent = m_swapChain->getSwapChainExtent();
    swapChainImageViews = m_swapChain->getSwapChainImageViews();


    m_swapChainFrameBuffers->initSwapChainFrameBuffers(m_swapChain.get(), renderPass);
    swapChainFramebuffers = m_swapChainFrameBuffers->getFramebuffers();
}


void Renderer::recordGammaCommandBuffer(Scene* scene, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    // 커맨드 버퍼 기록 시작
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // 렌더 패스 시작
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = gammaRenderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


    /*
     * 첫 번째 서브패스 - Object 렌더링
     */
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    const std::vector<std::shared_ptr<Object>>& objects = scene->getObjects();
    size_t objectCount = scene->getObjectCount();

    for (size_t i = 0; i < objectCount; i++) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[MAX_FRAMES_IN_FLIGHT * i + currentFrame], 0, nullptr);
        UniformBufferObject ubo{};
        ubo.model = objects[i]->getModelMatrix();
        ubo.view = scene->getViewMatrix();
        ubo.proj = scene->getProjMatrix(swapChainExtent);
        ubo.proj[1][1] *= -1;
        m_uniformBuffers[MAX_FRAMES_IN_FLIGHT * i + currentFrame]->updateUniformBuffer(&ubo, sizeof(ubo));
        objects[i]->draw(commandBuffer);
    }

        /*
     * 🔄 서브패스 전환
     */
    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

    /*
     * 🌟 두 번째 서브패스 - 감마 보정 (풀스크린 쿼드)
     */
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gammaGraphicsPipeline);

    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        gammaPipelineLayout,
        0,
        1,
        &gammaDescriptorSets[currentFrame],
        0,
        nullptr
    );

    // 풀스크린 쿼드 드로우

    // 유니폼버퍼 넣어주기
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);

    // 렌더 패스 종료
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record gamma command buffer!");
    }

}