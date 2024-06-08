#include "envCubemap.h"

void EnvCubemap::CreateEnvCubemap(uint32_t cubemapSize, vk::Format format, vk::ImageUsageFlags usage, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer)
{
    m_imageSize = cubemapSize;
    CreateCubemap(m_imageSize, format, usage, commandBuffer);

    vkn::Command::Begin(commandBuffer);
    for (int i = 0; i < 6; i++) {
        m_imageViewCreateInfo.subresourceRange.baseArrayLayer = i;
        vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                            m_bundle.image,
                                            vk::ImageLayout::eShaderReadOnlyOptimal,
                                            vk::ImageLayout::eColorAttachmentOptimal,
                                            vk::AccessFlagBits::eShaderRead,
                                            vk::AccessFlagBits::eColorAttachmentWrite,
                                            vk::PipelineStageFlagBits::eAllCommands,
                                            vk::PipelineStageFlagBits::eAllCommands,
                                            m_imageViewCreateInfo.subresourceRange);
    }

    commandBuffer.end();
    vkn::Command::SubmitAndWait(commandBuffer);

    CreateFramebuffer(cubemapPipeline, commandBuffer);
}

void EnvCubemap::CreateFramebuffer(const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer)
{
    vk::ImageView attachment;
    vk::FramebufferCreateInfo frameBufferCI({}, cubemapPipeline.m_renderPass, 1, &attachment, m_imageSize, m_imageSize, 1);

    for (uint32_t i = 0; i < 6; i++) {
        attachment = m_cubemapFaceImageViews[i];
        vkn::CheckResult(vkn::Device::Get().device.createFramebuffer(&frameBufferCI, nullptr, &m_framebuffers[i]));
    }
}

void EnvCubemap::DrawEnvCubemap(const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer)
{
    vkn::Command::Begin(commandBuffer);

    vk::Viewport viewport({}, {}, (float)m_imageSize, (float)m_imageSize, 0.0f, 1.0f);
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor({ 0, 0 }, { m_imageSize, m_imageSize });
    commandBuffer.setScissor(0, 1, &scissor);

    for (uint32_t face = 0; face < 6; face++) {
        DrawEnvCubemapFace(face, envCube, cubemapPipeline, commandBuffer);
    }

    for (int i = 0; i < 6; i++) {
        m_imageViewCreateInfo.subresourceRange.baseArrayLayer = i;
        vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                            m_bundle.image,
                                            vk::ImageLayout::eColorAttachmentOptimal,
                                            vk::ImageLayout::eShaderReadOnlyOptimal,
                                            vk::AccessFlagBits::eColorAttachmentWrite,
                                            vk::AccessFlagBits::eShaderRead,
                                            vk::PipelineStageFlagBits::eAllCommands,
                                            vk::PipelineStageFlagBits::eAllCommands,
                                            m_imageViewCreateInfo.subresourceRange);
    }
    m_bundle.descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    commandBuffer.end();
    vkn::Command::SubmitAndWait(commandBuffer);
}

void EnvCubemap::DrawEnvCubemapFace(uint32_t faceIndex, const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer)
{
    vk::ClearValue clearValues = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    vk::RenderPassBeginInfo renderPassBI(cubemapPipeline.m_renderPass, m_framebuffers[faceIndex], { { 0, 0 }, { m_imageSize, m_imageSize } }, 2, &clearValues);

    glm::mat4 viewMatrix;
    glm::mat4 projMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    switch (faceIndex) {
    case 0: // POS X
        viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    case 1: // NEG X
        viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    case 2: // POS Y
        viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        break;
    case 3: // NEG Y
        viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        break;
    case 4: // POS Z
        viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    case 5: // NEG Z
        viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    }

    commandBuffer.beginRenderPass(&renderPassBI, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, cubemapPipeline.m_pipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, cubemapPipeline.m_pipelineLayout, 0, 1, &cubemapPipeline.m_descriptorSets[0], 0, nullptr);

    vk::DeviceSize vertexOffsets[]{ 0 };
    m_pushConstants.view = viewMatrix;
    m_pushConstants.proj = projMatrix;
    commandBuffer.pushConstants(
        cubemapPipeline.m_pipelineLayout,
        vk::ShaderStageFlagBits::eVertex,
        0,
        sizeof(CubemapPushConstants),
        &m_pushConstants);
    commandBuffer.bindVertexBuffers(0, 1, &envCube.m_vertexBuffers[0]->Get().buffer, vertexOffsets);
    commandBuffer.bindIndexBuffer(envCube.m_indexBuffers[0]->Get().buffer, 0, vk::IndexType::eUint32);
    commandBuffer.drawIndexed(envCube.GetIndicesCount(0), envCube.GetInstanceCount(), 0, 0, 0);

    commandBuffer.endRenderPass();
}