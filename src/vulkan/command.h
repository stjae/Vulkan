#ifndef COMMAND_H
#define COMMAND_H

#include "device.h"

namespace vkn {
class Command
{
public:
    static void CreateCommandPool(vk::CommandPool& commandPool);
    static void AllocateCommandBuffer(const vk::CommandPool& commandPool, vk::CommandBuffer& commandBuffer);
    static void AllocateCommandBuffer(const vk::CommandPool& commandPool, std::vector<vk::CommandBuffer>& commandBuffers);
    static void Begin(const vk::CommandBuffer& commandBuffer, vk::CommandBufferUsageFlags flag = vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    static void End(const vk::CommandBuffer& commandBuffer);
    static void CopyBufferToBuffer(const vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, size_t size);
    static void CopyBufferToImage(const vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Image& dstImage, int width, int height, uint32_t layerIndex = 0);
    static void ChangeImageLayout(const vk::CommandBuffer& commandBuffer, const vk::Image& image, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout, vk::ImageSubresourceRange subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    static void SubmitAndWait(const vk::CommandBuffer& commandBuffer);
    static void SubmitAndWait(uint32_t count, vk::CommandBuffer* commandBuffers);
    static void BeginRenderPass(const vk::CommandBuffer& commandBuffer, vk::RenderPass& renderPass, vk::Framebuffer& frameBuffer, vk::Rect2D renderArea, std::vector<vk::ClearValue>& clearValues);

    template <size_t SIZE>
    static void AllocateCommandBuffer(const vk::CommandPool& commandPool, std::array<vk::CommandBuffer, SIZE>& commandBuffers)
    {
        vk::CommandBufferAllocateInfo allocateInfo;
        allocateInfo.commandPool = commandPool;
        allocateInfo.level = vk::CommandBufferLevel::ePrimary;
        allocateInfo.commandBufferCount = SIZE;

        for (int i = 0; i < SIZE; i++)
            commandBuffers[i] = vkn::Device::Get().device.allocateCommandBuffers(allocateInfo)[i];
    }
};
} // namespace vkn

#endif
