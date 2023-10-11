#include "commands.h"

void Command::CreateCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    poolInfo.setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());

    commandPool = device.createCommandPool(poolInfo);
    Log(debug, fmt::terminal_color::bright_green, "created command pool");
}

void Command::CreateCommandBuffer()
{
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.setCommandPool(commandPool);
    allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocateInfo.setCommandBufferCount(1);

    for (int i = 0; i < swapchainDetails.frames.size(); ++i) {
        swapchainDetails.frames[i].commandBuffer = device.allocateCommandBuffers(allocateInfo)[0];
        Log(debug, fmt::terminal_color::bright_green, "allocated command buffer for frame {}", i);
    }

    mainCommandBuffer = device.allocateCommandBuffers(allocateInfo)[0];
    Log(debug, fmt::terminal_color::bright_green, "allocated main command buffer");
}

void Command::RecordDrawCommands(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
{
    vk::CommandBufferBeginInfo beginInfo;
    commandBuffer.begin(beginInfo);

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.setRenderPass(renderPass);
    renderPassInfo.setFramebuffer(swapchainDetails.frames[imageIndex].framebuffer);
    vk::Rect2D renderArea(0, 0);
    renderArea.setExtent(swapchainDetails.extent);
    renderPassInfo.setRenderArea(renderArea);
    vk::ClearValue clearColor = { std::array<float, 4>{ 0.5f, 0.5f, 0.5f, 1.0f } };
    renderPassInfo.setClearValueCount(1);
    renderPassInfo.setPClearValues(&clearColor);

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();

    commandBuffer.end();
}

Command::~Command()
{
    device.destroyCommandPool(commandPool);
}