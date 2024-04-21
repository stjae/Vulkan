#ifndef COMMAND_H
#define COMMAND_H

#include "device.h"

namespace vkn {
class Command
{
public:
    static void CreateCommandPool(vk::CommandPool& commandPool);
    static void AllocateCommandBuffer(const vk::CommandPool& commandPool, vk::CommandBuffer& commandBuffer);
    static void Begin(vk::CommandBuffer& commandBuffer, vk::CommandBufferUsageFlags flag = vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    static void CopyBufferToBuffer(vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, size_t size);
    static void CopyBufferToImage(vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Image& dstImage, int width, int height, uint32_t layerIndex = 0);
    static void SetImageMemoryBarrier(vk::CommandBuffer& commandBuffer, const vk::Image& image, vk::DescriptorImageInfo& descriptorImageInfo, vk::ImageLayout srcImageLayout = vk::ImageLayout::eUndefined, vk::ImageLayout dstImageLayout = vk::ImageLayout::eUndefined, vk::AccessFlags srcAccessFlags = {}, vk::AccessFlags dstAccessFlags = {}, vk::PipelineStageFlags srcPipelineStageFlags = vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlags dstPipelineStageFlags = vk::PipelineStageFlagBits::eAllCommands, vk::ImageSubresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    static void SetImageMemoryBarrier(vk::CommandBuffer& commandBuffer, const vk::Image& image, vk::ImageLayout srcImageLayout = vk::ImageLayout::eUndefined, vk::ImageLayout dstImageLayout = vk::ImageLayout::eUndefined, vk::AccessFlags srcAccessFlags = {}, vk::AccessFlags dstAccessFlags = {}, vk::PipelineStageFlags srcPipelineStageFlags = vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlags dstPipelineStageFlags = vk::PipelineStageFlagBits::eAllCommands, vk::ImageSubresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    static void Submit(vk::CommandBuffer* commandBuffers, uint32_t count);
};
} // namespace vkn

#endif
