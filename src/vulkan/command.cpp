#include "command.h"
#include "device.h"
#include "image.h"

void vkn::Command::CreateCommandPool(vk::CommandPool& commandPool)
{
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = vkn::Device::Get().graphicsFamilyIndex.value();

    commandPool = vkn::Device::Get().device.createCommandPool(poolInfo);
}

void vkn::Command::AllocateCommandBuffer(const vk::CommandPool& commandPool, vk::CommandBuffer& commandBuffer)
{
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = commandPool;
    allocateInfo.level = vk::CommandBufferLevel::ePrimary;
    allocateInfo.commandBufferCount = 1;

    commandBuffer = vkn::Device::Get().device.allocateCommandBuffers(allocateInfo)[0];
}

void vkn::Command::AllocateCommandBuffer(const vk::CommandPool& commandPool, std::vector<vk::CommandBuffer>& commandBuffers)
{
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = commandPool;
    allocateInfo.level = vk::CommandBufferLevel::ePrimary;
    allocateInfo.commandBufferCount = commandBuffers.size();

    commandBuffers = vkn::Device::Get().device.allocateCommandBuffers(allocateInfo);
}

void vkn::Command::Begin(const vk::CommandBuffer& commandBuffer, vk::CommandBufferUsageFlags flag)
{
    vk::CommandBufferBeginInfo beginInfo(flag, {});
    vkn::CheckResult(commandBuffer.begin(&beginInfo));
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

void vkn::Command::SetImageMemoryBarrier(const vk::CommandBuffer& commandBuffer,
                                         const vk::Image& image,
                                         vk::DescriptorImageInfo& descriptorImageInfo,
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
    descriptorImageInfo.imageLayout = dstImageLayout;

    commandBuffer.pipelineBarrier(srcPipelineStageFlags, dstPipelineStageFlags, {}, 0, nullptr, 0, nullptr, 1, &barrier);
}

void vkn::Command::SetImageMemoryBarrier(const vk::CommandBuffer& commandBuffer,
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

void vkn::Command::Submit(const vk::CommandBuffer& commandBuffer)
{
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkn::Device::Get().graphicsQueue.submit(submitInfo);
    vkn::Device::Get().graphicsQueue.waitIdle();
}

void vkn::Command::Submit(const std::vector<vk::CommandBuffer>& commandBuffers)
{
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    vkn::Device::Get().graphicsQueue.submit(submitInfo);
    vkn::Device::Get().graphicsQueue.waitIdle();
}
