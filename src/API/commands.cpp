#include "commands.h"

void Command::CreateCommandPool(const char* user)
{
    user_ = user;
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = Queue::GetGraphicsQueueFamilyIndex();

    commandPool_ = Device::GetHandle().device.createCommandPool(poolInfo);
    if (user_)
        Log(debug, fmt::terminal_color::bright_green, "{}: {}", user_, "created command pool");
    else
        Log(debug, fmt::terminal_color::bright_green, "created command pool");
}

void Command::AllocateCommandBuffer(vk::CommandBuffer& commandBuffer)
{
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = commandPool_;
    allocateInfo.level = vk::CommandBufferLevel::ePrimary;
    allocateInfo.commandBufferCount = 1;

    commandBuffer = Device::GetHandle().device.allocateCommandBuffers(allocateInfo)[0];
    Log(debug, fmt::terminal_color::bright_green, "allocated command buffer");
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

void Command::RecordCopyCommands(vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, size_t size)
{
    vk::BufferCopy copyRegion(0, 0, size);
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
}

void Command::RecordCopyCommands(vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Image& dstImage, int width, int height)
{
    vk::ImageSubresourceLayers subResLayer(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
    vk::BufferImageCopy copyRegion(0, 0, 0, subResLayer, { 0, 0, 0 }, { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 });
    commandBuffer.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);
}

void Command::Transit(vk::CommandBuffer& commandBuffer, Image* image, vk::ImageLayout srcImageLayout, vk::ImageLayout dstImageLayout, vk::AccessFlags srcAccessFlags, vk::AccessFlags dstAccessFlags, vk::PipelineStageFlags srcPipelineStageFlags, vk::PipelineStageFlags dstPipelineStageFlags)
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
    barrier.image = image->GetHandle().image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    commandBuffer.pipelineBarrier(srcStage, dstStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);
    image->SetInfo(dstImageLayout);
}

void Command::TransitImageLayout(Image* image, vk::ImageLayout srcImageLayout, vk::ImageLayout dstImageLayout, vk::AccessFlags srcAccessFlags, vk::AccessFlags dstAccessFlags, vk::PipelineStageFlags srcPipelineStageFlags, vk::PipelineStageFlags dstPipelineStageFlags)
{
    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, {});
    commandBuffers_.back().begin(beginInfo);

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
    barrier.image = image->GetHandle().image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    commandBuffers_.back().pipelineBarrier(srcStage, dstStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);
    commandBuffers_.back().end();
}

void Command::Submit(vk::CommandBuffer* commandBuffers, uint32_t count)
{
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = count;
    submitInfo.pCommandBuffers = commandBuffers;

    Queue::GetHandle().graphicsQueue.submit(submitInfo);
    Queue::GetHandle().graphicsQueue.waitIdle();
}

void Command::Submit()
{
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = commandBuffers_.size();
    submitInfo.pCommandBuffers = commandBuffers_.data();

    Queue::GetHandle().graphicsQueue.submit(submitInfo);
    Queue::GetHandle().graphicsQueue.waitIdle();
}

Command::~Command()
{
    Device::GetHandle().device.destroyCommandPool(commandPool_);
    if (user_)
        Log(debug, fmt::terminal_color::bright_yellow, "{}: {}", user_, "destroyed command pool");
    else
        Log(debug, fmt::terminal_color::bright_yellow, "destroyed command pool");
}