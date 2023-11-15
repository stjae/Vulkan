#include "commands.h"

void Command::CreateCommandPool(const char* usage)
{
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = device.queueFamilyIndices.graphicsFamily.value();

    commandPool = device.vkDevice.createCommandPool(poolInfo);
    Log(debug, fmt::terminal_color::bright_green, "created command pool for {}", usage);
}

void Command::CreateCommandBuffer(vk::CommandBuffer& commandBuffer)
{
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = commandPool;
    allocateInfo.level = vk::CommandBufferLevel::ePrimary;
    allocateInfo.commandBufferCount = 1;

    commandBuffer = device.vkDevice.allocateCommandBuffers(allocateInfo)[0];
    Log(debug, fmt::terminal_color::bright_green, "allocated command buffer");
}

void Command::RecordDrawCommands(const GraphicsPipeline& pipeline, const vk::CommandBuffer& commandBuffer, uint32_t imageIndex, std::unique_ptr<Scene>& scene, ImDrawData* imDrawData)
{
    vk::CommandBufferBeginInfo beginInfo;
    commandBuffer.begin(beginInfo);
    {
        vk::ImageMemoryBarrier barrier;

        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.oldLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
        barrier.srcQueueFamilyIndex = device.queueFamilyIndices.graphicsFamily.value();
        barrier.dstQueueFamilyIndex = device.queueFamilyIndices.graphicsFamily.value();
        barrier.image = pipeline.swapchainDetail.frames[imageIndex].swapchainVkImage;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                      vk::DependencyFlagBits::eByRegion,
                                      0, nullptr, 0, nullptr, 1, &barrier);
    }
    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = pipeline.vkRenderPass;
    renderPassInfo.framebuffer = pipeline.swapchainDetail.frames[imageIndex].framebuffer;
    vk::Rect2D renderArea(0, 0);
    renderArea.extent = pipeline.swapchainDetail.extent;
    renderPassInfo.renderArea = renderArea;
    vk::ClearValue clearValue;
    clearValue.color = { std::array<float, 4>{ 0.1f, 0.1f, 0.1f, 1.0f } };
    vk::ClearValue depthClear;
    depthClear.depthStencil.depth = 1.0f;
    renderPassInfo.clearValueCount = 2;
    vk::ClearValue clearValues[] = { clearValue, depthClear };
    renderPassInfo.pClearValues = &clearValues[0];

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.vkPipelineLayout, 0, pipeline.swapchainDetail.frames[imageIndex].descriptorSets.size(), pipeline.swapchainDetail.frames[imageIndex].descriptorSets.data(), 0, 0);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.vkPipeline);

    vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(pipeline.swapchainDetail.extent.width);
    viewport.height = static_cast<float>(pipeline.swapchainDetail.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = pipeline.swapchainDetail.extent;

    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, scissor);

    for (auto& mesh : scene->meshes) {

        vk::Buffer vertexBuffers[] = { mesh->vertexBuffer->vkBuffer };
        vk::DeviceSize offsets[] = { 0 };

        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
        commandBuffer.bindIndexBuffer(mesh->indexBuffer->vkBuffer, 0, vk::IndexType::eUint16);

        commandBuffer.drawIndexed(static_cast<uint32_t>(mesh->indices.size()), 1, 0, 0, 0);
    }

    ImGui_ImplVulkan_RenderDrawData(imDrawData, commandBuffer);

    commandBuffer.endRenderPass();
    {
        vk::ImageMemoryBarrier barrier;

        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
        barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier.srcQueueFamilyIndex = device.queueFamilyIndices.graphicsFamily.value();
        barrier.dstQueueFamilyIndex = device.queueFamilyIndices.graphicsFamily.value();
        barrier.image = pipeline.swapchainDetail.frames[imageIndex].swapchainVkImage;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe,
                                      vk::DependencyFlagBits::eByRegion,
                                      0, nullptr, 0, nullptr, 1, &barrier);
    }
    commandBuffer.end();
}

void Command::RecordCopyCommands(const vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, size_t size)
{
    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, {});

    commandBuffer.begin(beginInfo);

    vk::BufferCopy copyRegion(0, 0, size);

    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
    commandBuffer.end();
}

void Command::RecordCopyCommands(const vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Image& dstImage, const int width, const int height, size_t size)
{
    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, {});

    commandBuffer.begin(beginInfo);

    vk::ImageSubresourceLayers subResLayer(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
    vk::BufferImageCopy copyRegion(0, 0, 0, subResLayer, { 0, 0, 0 }, { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 });

    commandBuffer.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);
    commandBuffer.end();
}

Command::~Command()
{
    device.vkDevice.destroyCommandPool(commandPool);
    Log(debug, fmt::v9::terminal_color::bright_yellow, "command pool destroyed");
}