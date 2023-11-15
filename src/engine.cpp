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
        command.CreateCommandBuffer(frame.commandBuffer);
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

void GraphicsEngine::CreateDepthImage()
{
    vk::Extent3D depthImageExtent{ swapchain.detail.extent.width, swapchain.detail.extent.height, 1 };
    vk::ImageCreateInfo imageCreateInfo({}, vk::ImageType::e2D, vk::Format::eD32Sfloat, depthImageExtent, 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment);
}

void GraphicsEngine::UpdateFrame(uint32_t imageIndex, Camera& camera, std::unique_ptr<Scene>& scene)
{
    camera.matrix.model = glm::mat4(1.0f);
    camera.matrix.view = glm::lookAt(camera.pos, camera.at, camera.up);
    camera.matrix.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapchain.detail.extent.width) / static_cast<float>(swapchain.detail.extent.height), 0.1f, 100.0f);
    camera.matrix.proj[1][1] *= -1;

    ubo.model = camera.matrix.model;
    ubo.view = camera.matrix.view;
    ubo.proj = camera.matrix.proj;
    ubo.eye = camera.pos;

    memcpy(swapchain.detail.frames[imageIndex].matrixUniformBufferMemoryLocation, &(ubo), sizeof(UBO));
    memcpy(swapchain.detail.frames[imageIndex].lightUniformBufferMemoryLocation, scene->pointLight.get(), sizeof(Light));

    swapchain.detail.frames[imageIndex].WriteDescriptorSet(device.vkDevice);

    for (auto& mesh : scene->meshes) {
        vk::WriteDescriptorSet descriptorWrites;
        descriptorWrites.dstSet = swapchain.detail.frames[imageIndex].descriptorSets[0];
        descriptorWrites.dstBinding = 2;
        descriptorWrites.dstArrayElement = 0;
        descriptorWrites.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites.descriptorCount = 1;
        descriptorWrites.pImageInfo = &mesh->textureImage->imageInfo;

        device.vkDevice.updateDescriptorSets(descriptorWrites, nullptr);
    }
}

void GraphicsEngine::Prepare(std::unique_ptr<Scene>& scene)
{
    scene->CreateResource(device);

    Command command(device);
    command.CreateCommandPool("copying buffers");

    std::vector<vk::CommandBuffer> commandBuffers;
    for (int j = 0; j < scene->meshes.size(); ++j) {

        auto& mesh = scene->meshes[j];

        commandBuffers.emplace_back();
        command.CreateCommandBuffer(commandBuffers.back());
        command.RecordCopyCommands(commandBuffers.back(), mesh->vertexStagingBuffer->vkBuffer, mesh->vertexBuffer->vkBuffer, sizeof(mesh->vertices[0]) * mesh->vertices.size());
        commandBuffers.emplace_back();
        command.CreateCommandBuffer(commandBuffers.back());
        command.RecordCopyCommands(commandBuffers.back(), mesh->indexStagingBuffer->vkBuffer, mesh->indexBuffer->vkBuffer, sizeof(mesh->indices[0]) * mesh->indices.size());
        commandBuffers.emplace_back();
        command.CreateCommandBuffer(commandBuffers.back());
        mesh->textureImage->TransitImageLayout(commandBuffers.back(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    }

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    device.vkGraphicsQueue.submit(submitInfo);
    device.vkGraphicsQueue.waitIdle();

    commandBuffers.clear();

    for (int i = 0; i < scene->meshes.size(); ++i) {

        auto& mesh = scene->meshes[i];

        commandBuffers.emplace_back();
        command.CreateCommandBuffer(commandBuffers.back());
        command.RecordCopyCommands(commandBuffers.back(), mesh->textureStagingBuffer->vkBuffer, mesh->textureImage->image, mesh->textureWidth, mesh->textureHeight, mesh->textureSize);
    }

    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    device.vkGraphicsQueue.submit(submitInfo);
    device.vkGraphicsQueue.waitIdle();

    for (auto& mesh : scene->meshes) {

        mesh->textureImage->TransitImageLayout(commandBuffers.back(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        submitInfo.commandBufferCount = commandBuffers.size();
        submitInfo.pCommandBuffers = commandBuffers.data();

        device.vkGraphicsQueue.submit(submitInfo);
        device.vkGraphicsQueue.waitIdle();

        vk::DescriptorImageInfo imageInfo(mesh->textureImage->sampler, mesh->textureImage->imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
        mesh->textureImage->imageInfo = imageInfo;
        mesh->DestroyStagingBuffer();
    }
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
        command.CreateCommandBuffer(frame.commandBuffer);
    }
}

GraphicsEngine::~GraphicsEngine()
{
    device.vkDevice.waitIdle();
}
