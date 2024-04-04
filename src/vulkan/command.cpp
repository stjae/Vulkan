#include "command.h"
#include "device.h"
#include "image.h"

void vkn::Command::CreateCommandPool(vk::CommandPool& commandPool)
{
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = vkn::Device::GetBundle().graphicsFamilyIndex.value();

    commandPool = vkn::Device::GetBundle().device.createCommandPool(poolInfo);
}

void vkn::Command::AllocateCommandBuffer(const vk::CommandPool& commandPool, vk::CommandBuffer& commandBuffer)
{
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = commandPool;
    allocateInfo.level = vk::CommandBufferLevel::ePrimary;
    allocateInfo.commandBufferCount = 1;

    commandBuffer = vkn::Device::GetBundle().device.allocateCommandBuffers(allocateInfo)[0];
}

void vkn::Command::Begin(vk::CommandBuffer& commandBuffer, vk::CommandBufferUsageFlags flag)
{
    vk::CommandBufferBeginInfo beginInfo(flag, {});
    commandBuffer.begin(&beginInfo);
}
void vkn::Command::CopyBufferToBuffer(vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, size_t size)
{
    vk::BufferCopy copyRegion(0, 0, size);
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
}

void vkn::Command::CopyBufferToImage(vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Image& dstImage, int width, int height)
{
    vk::ImageSubresourceLayers subResLayer(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
    vk::BufferImageCopy copyRegion(0, 0, 0, subResLayer, { 0, 0, 0 }, { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 });
    commandBuffer.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);
}

void vkn::Command::SetImageMemoryBarrier(vk::CommandBuffer& commandBuffer,
                                         Image& image,
                                         vk::ImageLayout srcImageLayout,
                                         vk::ImageLayout dstImageLayout,
                                         vk::AccessFlags srcAccessFlags,
                                         vk::AccessFlags dstAccessFlags,
                                         vk::PipelineStageFlags srcPipelineStageFlags,
                                         vk::PipelineStageFlags dstPipelineStageFlags,
                                         vk::ImageSubresourceRange subresourceRange)
{
    vk::ImageMemoryBarrier barrier;
    barrier.srcAccessMask = srcAccessFlags;
    barrier.dstAccessMask = dstAccessFlags;
    barrier.oldLayout = srcImageLayout;
    barrier.newLayout = dstImageLayout;
    barrier.image = image.GetBundle().image;
    barrier.subresourceRange = subresourceRange;

    commandBuffer.pipelineBarrier(srcPipelineStageFlags, dstPipelineStageFlags, {}, 0, nullptr, 0, nullptr, 1, &barrier);
}

void vkn::Command::SetImageMemoryBarrier(vk::CommandBuffer& commandBuffer,
                                         const vk::Image& image,
                                         vk::ImageLayout srcImageLayout,
                                         vk::ImageLayout dstImageLayout,
                                         vk::AccessFlags srcAccessFlags,
                                         vk::AccessFlags dstAccessFlags,
                                         vk::PipelineStageFlags srcPipelineStageFlags,
                                         vk::PipelineStageFlags dstPipelineStageFlags,
                                         vk::ImageSubresourceRange subresourceRange)
{
    vk::ImageMemoryBarrier barrier;
    barrier.srcAccessMask = srcAccessFlags;
    barrier.dstAccessMask = dstAccessFlags;
    barrier.oldLayout = srcImageLayout;
    barrier.newLayout = dstImageLayout;
    barrier.image = image;
    barrier.subresourceRange = subresourceRange;

    commandBuffer.pipelineBarrier(srcPipelineStageFlags, dstPipelineStageFlags, {}, 0, nullptr, 0, nullptr, 1, &barrier);
}

void vkn::Command::Submit(vk::CommandBuffer* commandBuffers, uint32_t count)
{
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = count;
    submitInfo.pCommandBuffers = commandBuffers;

    vkn::Device::GetBundle().graphicsQueue.submit(submitInfo);
    vkn::Device::GetBundle().graphicsQueue.waitIdle();
}