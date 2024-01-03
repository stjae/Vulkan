#include "commands.h"

void Command::CreateCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = Queue::GetGraphicsQueueFamilyIndex();

    commandPool_ = Device::GetHandle().device.createCommandPool(poolInfo);
    Log(debug, fmt::terminal_color::bright_green, "created command pool");
}

void Command::AllocateCommandBuffer()
{
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = commandPool_;
    allocateInfo.level = vk::CommandBufferLevel::ePrimary;
    allocateInfo.commandBufferCount = 1;

    commandBuffers_.push_back(Device::GetHandle().device.allocateCommandBuffers(allocateInfo)[0]);
    Log(debug, fmt::terminal_color::bright_green, "allocated command buffer");
}

void Command::RecordCopyCommands(const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, size_t size)
{
    AllocateCommandBuffer();
    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
                                         {});
    commandBuffers_.back().begin(beginInfo);

    vk::BufferCopy copyRegion(0, 0, size);

    commandBuffers_.back().copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
    commandBuffers_.back().end();
}

void Command::RecordCopyCommands(const vk::Buffer& srcBuffer, const vk::Image& dstImage, int width, int height)
{
    AllocateCommandBuffer();
    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, {});
    commandBuffers_.back().begin(beginInfo);

    vk::ImageSubresourceLayers subResLayer(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
    vk::BufferImageCopy copyRegion(0, 0, 0, subResLayer, { 0, 0, 0 }, { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 });

    commandBuffers_.back().copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);
    commandBuffers_.back().end();
}

void Command::TransitImageLayout(const vk::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    AllocateCommandBuffer();
    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, {});
    commandBuffers_.back().begin(beginInfo);

    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;

    vk::ImageMemoryBarrier barrier;
    if (oldLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
        spdlog::error("unsupported layout transition");
    }
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    commandBuffers_.back().pipelineBarrier(srcStage, dstStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);
    commandBuffers_.back().end();
}

void Command::Submit()
{
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = commandBuffers_.size();
    submitInfo.pCommandBuffers = commandBuffers_.data();

    Queue::GetHandle().graphicsQueue.submit(submitInfo);
    Queue::GetHandle().graphicsQueue.waitIdle();

    commandBuffers_.clear();
}

Command::~Command()
{
    Device::GetHandle().device.destroyCommandPool(commandPool_);
    Log(debug, fmt::v9::terminal_color::bright_yellow, "command pool destroyed");
}