#include "commands.h"

void Command::CreateCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    commandPool = device.createCommandPool(poolInfo);
    Log(debug, fmt::terminal_color::bright_green, "created command pool");
}

void Command::CreateCommandBuffer(vk::CommandBuffer& commandBuffer)
{
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = commandPool;
    allocateInfo.level = vk::CommandBufferLevel::ePrimary;
    allocateInfo.commandBufferCount = 1;

    commandBuffer = device.allocateCommandBuffers(allocateInfo)[0];
    Log(debug, fmt::terminal_color::bright_green, "allocated command buffer");
}

void Command::RecordDrawCommands(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene)
{
    vk::CommandBufferBeginInfo beginInfo;
    commandBuffer.begin(beginInfo);

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapchainDetails.frames[imageIndex].framebuffer;
    vk::Rect2D renderArea(0, 0);
    renderArea.extent = swapchainDetails.extent;
    renderPassInfo.renderArea = renderArea;
    vk::ClearValue clearColor = { std::array<float, 4>{ 0.5f, 0.5f, 0.5f, 1.0f } };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    vk::Buffer vertexBuffers[] = { scene->m_triangleMesh.get()->m_vertexBuffer.buffer };
    vk::DeviceSize offsets[] = { 0 };
    commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

    for (glm::vec3 pos : scene->positions) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
        ObjectData objectData;
        objectData.model = model;
        commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(ObjectData), &objectData);
        commandBuffer.draw(3, 1, 0, 0);
    }

    commandBuffer.endRenderPass();
    commandBuffer.end();
}

void Command::RecordCopyCommands(vk::CommandBuffer commandBuffer, vk::Buffer srcBuffer, vk::Buffer dstBuffer, size_t size)
{
    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, {});

    commandBuffer.begin(beginInfo);

    vk::BufferCopy copyRegion(0, 0, size);

    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
    commandBuffer.end();
}

Command::~Command()
{
    device.destroyCommandPool(commandPool);
}