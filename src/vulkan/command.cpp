#include "command.h"

void Command::CreateCommandPool(vk::CommandPool& commandPool)
{
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = Device::GetBundle().graphicsComputeFamilyIndex.value();

    commandPool = Device::GetBundle().device.createCommandPool(poolInfo);
}

void Command::AllocateCommandBuffer(const vk::CommandPool& commandPool, vk::CommandBuffer& commandBuffer)
{
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = commandPool;
    allocateInfo.level = vk::CommandBufferLevel::ePrimary;
    allocateInfo.commandBufferCount = 1;

    commandBuffer = Device::GetBundle().device.allocateCommandBuffers(allocateInfo)[0];
}

void Command::Begin(vk::CommandBuffer& commandBuffer, vk::CommandBufferUsageFlags flag)
{
    vk::CommandBufferBeginInfo beginInfo(flag, {});
    commandBuffer.begin(&beginInfo);
}
void Command::CopyBufferToBuffer(vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, size_t size)
{
    vk::BufferCopy copyRegion(0, 0, size);
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
}

void Command::CopyBufferToImage(vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Image& dstImage, int width, int height)
{
    vk::ImageSubresourceLayers subResLayer(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
    vk::BufferImageCopy copyRegion(0, 0, 0, subResLayer, { 0, 0, 0 }, { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 });
    commandBuffer.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);
}

void Command::SetImageMemoryBarrier(vk::CommandBuffer& commandBuffer, Image& image, vk::ImageLayout srcImageLayout, vk::ImageLayout dstImageLayout, vk::AccessFlags srcAccessFlags, vk::AccessFlags dstAccessFlags, vk::PipelineStageFlags srcPipelineStageFlags, vk::PipelineStageFlags dstPipelineStageFlags)
{
    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;

    vk::ImageMemoryBarrier barrier;
    barrier.srcAccessMask = srcAccessFlags;
    barrier.dstAccessMask = dstAccessFlags;

    srcStage = srcPipelineStageFlags;
    dstStage = dstPipelineStageFlags;

    barrier.oldLayout = srcImageLayout;
    barrier.newLayout = dstImageLayout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image = image.GetBundle().image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    commandBuffer.pipelineBarrier(srcStage, dstStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);
    image.SetInfo(dstImageLayout);
}

void Command::SetImageMemoryBarrier(vk::CommandBuffer& commandBuffer, const vk::Image& image, vk::ImageLayout srcImageLayout, vk::ImageLayout dstImageLayout, vk::AccessFlags srcAccessFlags, vk::AccessFlags dstAccessFlags, vk::PipelineStageFlags srcPipelineStageFlags, vk::PipelineStageFlags dstPipelineStageFlags)
{
    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;

    vk::ImageMemoryBarrier barrier;
    barrier.srcAccessMask = srcAccessFlags;
    barrier.dstAccessMask = dstAccessFlags;

    srcStage = srcPipelineStageFlags;
    dstStage = dstPipelineStageFlags;

    barrier.oldLayout = srcImageLayout;
    barrier.newLayout = dstImageLayout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    commandBuffer.pipelineBarrier(srcStage, dstStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Command::Submit(vk::CommandBuffer* commandBuffers, uint32_t count)
{
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = count;
    submitInfo.pCommandBuffers = commandBuffers;

    Device::GetBundle().graphicsQueue.submit(submitInfo);
    Device::GetBundle().graphicsQueue.waitIdle();
}