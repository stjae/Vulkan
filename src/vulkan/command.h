// Wrapper class for Vulkan command management
// Vulkanコマンド管理用ラッパークラス

#ifndef COMMAND_H
#define COMMAND_H

#include "device.h"

namespace vkn {
class Command
{
    inline static vk::CommandPool s_commandPools[MAX_FRAMES_IN_FLIGHT];
    inline static vk::CommandBuffer s_commandBuffers[MAX_FRAMES_IN_FLIGHT];

public:
    static void CreateCommandPools();
    static void DestroyCommandPools();
    static void AllocateCommandBuffers();
    static void Begin(const vk::CommandBuffer& commandBuffer, vk::CommandBufferUsageFlags flag = vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    static void Begin(int currentFrame, vk::CommandBufferUsageFlags flag = vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    static void End(const vk::CommandBuffer& commandBuffer);
    static void End(int currentFrame);
    static void SubmitAndWait(const vk::CommandBuffer& commandBuffer);
    static void SubmitAndWait(int currentFrame);
    static void CopyBufferToBuffer(const vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, size_t size);
    static void CopyBufferToImage(const vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Image& dstImage, int width, int height, uint32_t layerIndex = 0);
    static void ChangeImageLayout(const vk::CommandBuffer& commandBuffer, const vk::Image& image, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout, vk::ImageSubresourceRange subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    static void BeginRenderPass(const vk::CommandBuffer& commandBuffer, vk::RenderPass& renderPass, vk::Framebuffer& frameBuffer, vk::Rect2D renderArea, std::vector<vk::ClearValue>& clearValues);
    static const vk::CommandBuffer& GetCommandBuffer(int currentFrame) { return s_commandBuffers[currentFrame]; }

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
