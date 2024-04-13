#include "envCubemap.h"

void EnvCubemap::CreateEnvCubemap(uint32_t cubemapSize, vk::Format format, vk::ImageUsageFlags usage, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer)
{
    imageSize_ = cubemapSize;

    CreateCubemap(imageSize_, format, usage, commandBuffer);

    for (int i = 0; i < 6; i++) {
        vkn::Command::Begin(commandBuffer);
        imageViewCreateInfo.subresourceRange.baseArrayLayer = i;
        vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                            imageBundle_.image,
                                            vk::ImageLayout::eShaderReadOnlyOptimal,
                                            vk::ImageLayout::eColorAttachmentOptimal,
                                            vk::AccessFlagBits::eShaderRead,
                                            vk::AccessFlagBits::eColorAttachmentWrite,
                                            vk::PipelineStageFlagBits::eAllCommands,
                                            vk::PipelineStageFlagBits::eAllCommands,
                                            imageViewCreateInfo.subresourceRange);
        commandBuffer.end();
        vkn::Command::Submit(&commandBuffer, 1);
    }

    CreateFramebuffer(cubemapPipeline, commandBuffer);
}

void EnvCubemap::CreateFramebuffer(const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer)
{
    vk::ImageView attachment;
    vk::FramebufferCreateInfo frameBufferCI({}, cubemapPipeline.renderPass, 1, &attachment, imageSize_, imageSize_, 1);

    for (uint32_t i = 0; i < 6; i++) {
        attachment = cubemapFaceImageViews_[i];
        vkn::CheckResult(vkn::Device::GetBundle().device.createFramebuffer(&frameBufferCI, nullptr, &framebuffers_[i]));
    }
}

void EnvCubemap::DrawEnvCubemap(const Mesh& envCube, const vkn::Image& envMap, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer)
{
    vkn::Command::Begin(commandBuffer);

    UpdateDescriptorSets(cubemapPipeline, envMap);

    vk::Viewport viewport({}, {}, (float)imageSize_, (float)imageSize_, 0.0f, 1.0f);
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor({ 0, 0 }, { imageSize_, imageSize_ });
    commandBuffer.setScissor(0, 1, &scissor);

    for (uint32_t face = 0; face < 6; face++) {
        DrawEnvCubemapFace(face, envCube, cubemapPipeline, commandBuffer);
    }

    commandBuffer.end();
    vkn::Command::Submit(&commandBuffer, 1);

    for (int i = 0; i < 6; i++) {
        vkn::Command::Begin(commandBuffer);
        imageViewCreateInfo.subresourceRange.baseArrayLayer = i;
        vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                            imageBundle_.image,
                                            vk::ImageLayout::eColorAttachmentOptimal,
                                            vk::ImageLayout::eShaderReadOnlyOptimal,
                                            vk::AccessFlagBits::eColorAttachmentWrite,
                                            vk::AccessFlagBits::eShaderRead,
                                            vk::PipelineStageFlagBits::eAllCommands,
                                            vk::PipelineStageFlagBits::eAllCommands,
                                            imageViewCreateInfo.subresourceRange);
        commandBuffer.end();
        vkn::Command::Submit(&commandBuffer, 1);
    }
    imageBundle_.descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
}

void EnvCubemap::UpdateDescriptorSets(const vkn::Pipeline& cubemapPipeline, const vkn::Image& envMap)
{
    std::vector<vk::WriteDescriptorSet> writes = {
        { cubemapPipeline.descriptorSets[0], 0, 0, 1, vk::DescriptorType::eCombinedImageSampler, &envMap.GetBundle().descriptorImageInfo }
    };

    vkn::Device::GetBundle().device.updateDescriptorSets(writes, nullptr);
}

void EnvCubemap::DrawEnvCubemapFace(uint32_t faceIndex, const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer)
{
    vk::ClearValue clearValues = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    vk::RenderPassBeginInfo renderPassBI(cubemapPipeline.renderPass, framebuffers_[faceIndex], { { 0, 0 }, { imageSize_, imageSize_ } }, 2, &clearValues);

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
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, cubemapPipeline.pipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, cubemapPipeline.pipelineLayout, 0, 1, &cubemapPipeline.descriptorSets[0], 0, nullptr);

    vk::DeviceSize vertexOffsets[]{ 0 };
    pushConstants_.view = viewMatrix;
    pushConstants_.proj = projMatrix;
    commandBuffer.pushConstants(
        cubemapPipeline.pipelineLayout,
        vk::ShaderStageFlagBits::eVertex,
        0,
        sizeof(CubemapPushConstants),
        &pushConstants_);
    commandBuffer.bindVertexBuffers(0, 1, &envCube.vertexBuffers[0]->GetBundle().buffer, vertexOffsets);
    commandBuffer.bindIndexBuffer(envCube.indexBuffers[0]->GetBundle().buffer, 0, vk::IndexType::eUint32);
    commandBuffer.drawIndexed(envCube.GetIndicesCount(0), envCube.GetInstanceCount(), 0, 0, 0);

    commandBuffer.endRenderPass();
}