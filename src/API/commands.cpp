#include "commands.h"

void Command::CreateCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = device.queueFamilyIndices.graphicsFamily.value();

    commandPool = device.vkDevice.createCommandPool(poolInfo);
    Log(debug, fmt::terminal_color::bright_green, "created command pool");
}

void Command::CreateCommandBuffer(vk::CommandBuffer& commandBuffer)
{
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = commandPool;
    allocateInfo.level = vk::CommandBufferLevel::ePrimary;
    allocateInfo.commandBufferCount = 1;

    commandBuffer = device.vkDevice.allocateCommandBuffers(allocateInfo)[0];
    Log(debug, fmt::terminal_color::bright_green, "allocated command buffer");
}

void Command::RecordDrawCommands(GraphicsPipeline& pipeline, vk::CommandBuffer commandBuffer, uint32_t imageIndex, std::unique_ptr<Scene>& scene)
{
    vk::CommandBufferBeginInfo beginInfo;
    commandBuffer.begin(beginInfo);

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = pipeline.vkRenderPass;
    renderPassInfo.framebuffer = pipeline.swapchainDetail.frames[imageIndex].framebuffer;
    vk::Rect2D renderArea(0, 0);
    renderArea.extent = pipeline.swapchainDetail.extent;
    renderPassInfo.renderArea = renderArea;
    vk::ClearValue clearColor = { std::array<float, 4>{ 0.5f, 0.5f, 0.5f, 1.0f } };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.vkPipelineLayout, 0, pipeline.swapchainDetail.frames[imageIndex].descriptorSet, nullptr);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.vkPipeline);

    for (auto& mesh : scene->meshes) {

        vk::Buffer vertexBuffers[] = { mesh->vertexBuffer->vkBuffer };
        vk::DeviceSize offsets[] = { 0 };

        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
        commandBuffer.bindIndexBuffer(mesh->indexBuffer->vkBuffer, 0, vk::IndexType::eUint16);

        for (glm::vec3 pos : scene->positions) {
            // glm::mat4 model = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
            ObjectData objectData;
            objectData.model = model;
            commandBuffer.pushConstants(pipeline.vkPipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(ObjectData), &objectData);
            commandBuffer.drawIndexed(static_cast<uint32_t>(mesh->indices.size()), 1, 0, 0, 0);
        }
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
    device.vkDevice.destroyCommandPool(commandPool);
}