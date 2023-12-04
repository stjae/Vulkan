#include "engine.h"

GraphicsEngine::GraphicsEngine(int width, int height, GLFWwindow* window, std::unique_ptr<Scene>& scene)
    : device(window), swapchain(device, pipeline.vkRenderPass),
      pipeline(device.vkDevice, swapchain.detail, scene), command(device)
{
    this->window = window;
    this->width = width;
    this->height = height;

    swapchain.CreateSwapchain(this->window, device);
    pipeline.CreatePipeline();
    swapchain.CreateFrameBuffer();

    command.CreateCommandPool("swapchain frames");
    for (auto& frame : swapchain.detail.frames) {
        command.AllocateCommandBuffer(frame.commandBuffer);
    }

    maxFrameNumber = static_cast<int>(swapchain.detail.frames.size());
    frameIndex = 0;

    swapchain.PrepareFrames();

    pipeline.CreateDescriptorPool();
    for (auto& frame : swapchain.detail.frames) {
        pipeline.descriptor.AllocateSet(frame.descriptorSets);
    }
}

void GraphicsEngine::InitSwapchainImages()
{
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    for (auto& frame : swapchain.detail.frames) {
        frame.commandBuffer.begin(&beginInfo);

        vk::ImageMemoryBarrier barrier;

        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier.srcQueueFamilyIndex = device.queueFamilyIndices.graphicsFamily.value();
        barrier.dstQueueFamilyIndex = device.queueFamilyIndices.graphicsFamily.value();
        barrier.image = frame.swapchainVkImage;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        frame.commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe, {}, 0, nullptr, 0, nullptr, 1, &barrier);

        frame.commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &frame.commandBuffer;

        device.vkGraphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
        device.vkDevice.waitIdle();
    }
}

void GraphicsEngine::UpdateFrame(uint32_t imageIndex, Camera& camera, std::unique_ptr<Scene>& scene)
{
    camera.matrix.view = glm::lookAt(camera.pos, camera.at, camera.up);
    camera.matrix.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapchain.detail.extent.width) / static_cast<float>(swapchain.detail.extent.height), 0.1f, 100.0f);

    std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos;

    for (int i = 0; i < scene->meshes.size(); i++) {

        auto& mesh = scene->meshes[i];

        mesh->ubo.view = camera.matrix.view;
        mesh->ubo.proj = camera.matrix.proj;
        mesh->ubo.eye = camera.pos;

        memcpy(mesh->matrixUniformBuffer->memory.memoryLocation, &(mesh->ubo), sizeof(UBO));

        descriptorBufferInfos.push_back(mesh->matrixUniformBuffer->descriptorBufferInfo);
    }

    vk::WriteDescriptorSet matrixWriteInfo(swapchain.detail.frames[imageIndex].descriptorSets[0], 0, 0, 2, vk::DescriptorType::eUniformBuffer, nullptr, descriptorBufferInfos.data(), nullptr, nullptr);
    device.vkDevice.updateDescriptorSets(matrixWriteInfo, nullptr);

    vk::WriteDescriptorSet descriptorWrites;
    descriptorWrites.dstSet = swapchain.detail.frames[imageIndex].descriptorSets[0];
    descriptorWrites.dstBinding = 2;
    descriptorWrites.dstArrayElement = 0;
    descriptorWrites.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrites.descriptorCount = 2;
    std::vector<vk::DescriptorImageInfo> infos;
    for (auto& mesh : scene->meshes) {
        infos.push_back(mesh->textureImage->imageInfo);
    }
    descriptorWrites.pImageInfo = infos.data();
    device.vkDevice.updateDescriptorSets(descriptorWrites, nullptr);
}

void GraphicsEngine::Prepare(std::unique_ptr<Scene>& scene)
{
    scene->CreateResource(device);

    Command command(device);
    command.CreateCommandPool("copying buffers");

    for (int j = 0; j < scene->meshes.size(); ++j) {

        auto& mesh = scene->meshes[j];

        command.RecordCopyCommands(mesh->vertexStagingBuffer->vkBuffer, mesh->vertexBuffer->vkBuffer, sizeof(mesh->vertices[0]) * mesh->vertices.size());
        command.RecordCopyCommands(mesh->indexStagingBuffer->vkBuffer, mesh->indexBuffer->vkBuffer, sizeof(mesh->indices[0]) * mesh->indices.size());
        command.TransitImageLayout(mesh->textureImage->image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    }

    command.Submit();

    for (int i = 0; i < scene->meshes.size(); ++i) {

        auto& mesh = scene->meshes[i];

        command.RecordCopyCommands(mesh->textureStagingBuffer->vkBuffer, mesh->textureImage->image, mesh->textureWidth, mesh->textureHeight, mesh->textureSize);
    }

    command.Submit();

    for (auto& mesh : scene->meshes) {

        command.TransitImageLayout(mesh->textureImage->image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        mesh->textureImage->SetTextureImageInfo(vk::ImageLayout::eShaderReadOnlyOptimal);
        mesh->DestroyStagingBuffer();
    }

    command.Submit();
}

void GraphicsEngine::Render(std::unique_ptr<Scene>& scene, ImDrawData* imDrawData, Camera& camera)
{
    auto resultWaitFence = device.vkDevice.waitForFences(1, &swapchain.detail.frames[frameIndex].inFlight, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    auto acquiredImage = device.vkDevice.acquireNextImageKHR(swapchain.vkSwapchain, UINT64_MAX, swapchain.detail.frames[frameIndex].imageAvailable, nullptr);

    if (acquiredImage.result == vk::Result::eErrorOutOfDateKHR ||
        acquiredImage.result == vk::Result::eSuboptimalKHR) {
        RecreateSwapchain();
        InitSwapchainImages();
        return;
    }

    auto resultResetFence = device.vkDevice.resetFences(1, &swapchain.detail.frames[frameIndex].inFlight);

    imageIndex = acquiredImage.value;

    UpdateFrame(imageIndex, camera, scene);

    vk::CommandBuffer commandBuffer = swapchain.detail.frames[frameIndex].commandBuffer;

    commandBuffer.reset();
    command.RecordDrawCommands(pipeline, commandBuffer, imageIndex, scene, imDrawData);

    vk::SubmitInfo submitInfo;
    vk::Semaphore waitSemaphores[] = { swapchain.detail.frames[frameIndex].imageAvailable };
    vk::PipelineStageFlags waitStage[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vk::Semaphore signalSemaphores[] = { swapchain.detail.frames[frameIndex].renderFinished };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    device.vkGraphicsQueue.submit(submitInfo, swapchain.detail.frames[frameIndex].inFlight);

    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    vk::SwapchainKHR swapchains[] = { swapchain.vkSwapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    vk::Result resultPresent;
    try {
        resultPresent = device.vkPresentQueue.presentKHR(presentInfo);
    } catch (vk::OutOfDateKHRError error) {
        RecreateSwapchain();
        InitSwapchainImages();
        return;
    }

    frameIndex = (frameIndex + 1) % maxFrameNumber;
}

void GraphicsEngine::RecreateSwapchain()
{
    int width = 0;
    int height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    device.vkDevice.waitIdle();
    for (auto& frame : swapchain.detail.frames) {
        device.vkDevice.freeCommandBuffers(command.commandPool, frame.commandBuffer);
    }
    swapchain.DestroySwapchain();
    device.vkDevice.destroyCommandPool(command.commandPool);

    swapchain.CreateSwapchain(window, device);
    swapchain.CreateFrameBuffer();

    swapchain.PrepareFrames();

    command.CreateCommandPool("new swapchain frames");
    for (auto& frame : swapchain.detail.frames) {
        command.AllocateCommandBuffer(frame.commandBuffer);
    }
}

GraphicsEngine::~GraphicsEngine()
{
    device.vkDevice.waitIdle();
}
