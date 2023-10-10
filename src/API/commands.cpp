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

    commandBuffer = device.allocateCommandBuffers(allocateInfo)[0];
    Log(debug, fmt::terminal_color::bright_green, "allocated main command buffer");
}

Command::~Command()
{
    device.destroyCommandPool(commandPool);
}