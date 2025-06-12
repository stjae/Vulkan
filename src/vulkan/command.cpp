#include "command.h"
#include "device.h"
#include "image.h"
#include "swapchain.h"

void vkn::Command::CreateCommandPools()
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::CommandPoolCreateInfo poolInfo;
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolInfo.queueFamilyIndex = vkn::Device::Get().graphicsFamilyIndex.value();
        s_commandPools[i] = vkn::Device::Get().device.createCommandPool(poolInfo);
    }
}

void vkn::Command::DestroyCommandPools()
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkn::Device::Get().device.destroyCommandPool(s_commandPools[i]);
    }
}

void vkn::Command::AllocateCommandBuffers()
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::CommandBufferAllocateInfo allocateInfo;
        allocateInfo.commandPool = s_commandPools[i];
        allocateInfo.level = vk::CommandBufferLevel::ePrimary;
        allocateInfo.commandBufferCount = 1;

        s_commandBuffers[i] = vkn::Device::Get().device.allocateCommandBuffers(allocateInfo)[0];
    }
}

void vkn::Command::Begin(const vk::CommandBuffer& commandBuffer, vk::CommandBufferUsageFlags flag)
{
    vk::CommandBufferBeginInfo beginInfo(flag, {});
    vkn::CHECK_RESULT(commandBuffer.begin(&beginInfo));
}

void vkn::Command::Begin(int currentFrame, vk::CommandBufferUsageFlags flag)
{
    vk::CommandBufferBeginInfo beginInfo(flag, {});
    vkn::CHECK_RESULT(s_commandBuffers[currentFrame].begin(&beginInfo));
}

void vkn::Command::End(const vk::CommandBuffer& commandBuffer)
{
    commandBuffer.end();
}

void vkn::Command::End(int currentFrame)
{
    s_commandBuffers[currentFrame].end();
}

void vkn::Command::CopyBufferToBuffer(const vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, size_t size)
{
    vk::BufferCopy copyRegion(0, 0, size);
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
}

void vkn::Command::CopyBufferToImage(const vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Image& dstImage, int width, int height, uint32_t layerIndex)
{
    vk::ImageSubresourceLayers subResLayer(vk::ImageAspectFlagBits::eColor, 0, layerIndex, 1);
    vk::BufferImageCopy copyRegion(0, 0, 0, subResLayer, { 0, 0, 0 }, { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 });
    commandBuffer.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);
}

void vkn::Command::ChangeImageLayout(const vk::CommandBuffer& commandBuffer, const vk::Image& image, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout, vk::ImageSubresourceRange subresourceRange)
{
    vk::ImageMemoryBarrier barrier;
    barrier.image = image;
    barrier.oldLayout = oldImageLayout;
    barrier.newLayout = newImageLayout;
    barrier.subresourceRange = subresourceRange;
    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;

    switch (oldImageLayout) {
    case vk::ImageLayout::eUndefined:
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        break;
    case vk::ImageLayout::eColorAttachmentOptimal:
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        break;
    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eLateFragmentTests;
        break;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        srcStage = vk::PipelineStageFlagBits::eFragmentShader;
        break;
    case vk::ImageLayout::eTransferSrcOptimal:
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        break;
    case vk::ImageLayout::eTransferDstOptimal:
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        break;
    case vk::ImageLayout::ePresentSrcKHR:
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        srcStage = vk::PipelineStageFlagBits::eBottomOfPipe;
        break;
    default:
        break;
    }

    switch (newImageLayout) {
    case vk::ImageLayout::ePresentSrcKHR:
        barrier.dstAccessMask = vk::AccessFlagBits::eNone;
        dstStage = vk::PipelineStageFlagBits::eBottomOfPipe;
        break;
    case vk::ImageLayout::eTransferDstOptimal:
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
        break;
    case vk::ImageLayout::eTransferSrcOptimal:
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
        break;
    case vk::ImageLayout::eColorAttachmentOptimal:
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        break;
    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        dstStage = vk::PipelineStageFlagBits::eLateFragmentTests;
        break;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
        break;
    case vk::ImageLayout::eGeneral:
        barrier.dstAccessMask = vk::AccessFlagBits::eHostWrite;
        dstStage = vk::PipelineStageFlagBits::eAllCommands;
        break;
    default:
        break;
    }

    commandBuffer.pipelineBarrier(srcStage, dstStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);
}

void vkn::Command::SubmitAndWait(const vk::CommandBuffer& commandBuffer)
{
    vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &commandBuffer);
    vkn::CHECK_RESULT(vkn::Device::Get().device.resetFences(1, &vkn::Sync::GetCommandFence()));
    vkn::CHECK_RESULT(vkn::Device::Get().graphicsQueue.submit(1, &submitInfo, vkn::Sync::GetCommandFence()));
    vkn::CHECK_RESULT(vkn::Device::Get().device.waitForFences(1, &vkn::Sync::GetCommandFence(), VK_TRUE, UINT64_MAX));
}

void vkn::Command::SubmitAndWait(int currentFrame)
{
    vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &s_commandBuffers[currentFrame]);
    vkn::CHECK_RESULT(vkn::Device::Get().device.resetFences(1, &vkn::Sync::GetCommandFence()));
    vkn::CHECK_RESULT(vkn::Device::Get().graphicsQueue.submit(1, &submitInfo, vkn::Sync::GetCommandFence()));
    vkn::CHECK_RESULT(vkn::Device::Get().device.waitForFences(1, &vkn::Sync::GetCommandFence(), VK_TRUE, UINT64_MAX));
}

void vkn::Command::BeginRenderPass(const vk::CommandBuffer& commandBuffer, vk::RenderPass& renderPass, vk::Framebuffer& framebuffer, vk::Rect2D renderArea, std::vector<vk::ClearValue>& clearValues)
{
    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea = renderArea;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
}