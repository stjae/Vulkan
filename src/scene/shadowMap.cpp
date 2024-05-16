#include "shadowMap.h"

void ShadowMap::CreateFramebuffer()
{
    vk::ImageView attachment;
    attachment = m_bundle.imageView;

    vk::FramebufferCreateInfo frameBufferCI({}, shadowMapPipeline.m_renderPass, 1, &attachment, shadowMapSize, shadowMapSize, 1);

    vkn::CheckResult(vkn::Device::Get().device.createFramebuffer(&frameBufferCI, nullptr, &m_framebuffer));
}

void ShadowMap::CreateShadowMap(vk::CommandBuffer& commandBuffer)
{
    CreateImage({ shadowMapSize, shadowMapSize, 1 }, shadowMapDepthFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal, vkn::Image::s_clampSampler);

    vkn::Command::Begin(commandBuffer);
    vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                        m_bundle.image,
                                        {},
                                        vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                        {},
                                        vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });
    commandBuffer.end();
    vkn::Command::Submit(commandBuffer);

    m_imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    CreateImageView();

    CreateFramebuffer();
}

void ShadowMap::DrawShadowMap(vk::CommandBuffer& commandBuffer, std::vector<MeshModel>& meshes)
{
    vkn::Command::Begin(commandBuffer);
    vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                        m_bundle.image,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                        {},
                                        vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });

    vk::Viewport viewport({}, {}, (float)shadowMapSize, (float)shadowMapSize, 0.0f, 1.0f);
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor({ 0, 0 }, { shadowMapSize, shadowMapSize });
    commandBuffer.setScissor(0, 1, &scissor);

    vk::ClearValue clearValue;
    clearValue.depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

    vk::RenderPassBeginInfo renderPassBI(shadowMapPipeline.m_renderPass, m_framebuffer, { { 0, 0 }, { shadowMapSize, shadowMapSize } }, 1, &clearValue);
    commandBuffer.beginRenderPass(&renderPassBI, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, shadowMapPipeline.m_pipeline);
    int meshIndex = 0;
    vk::DeviceSize vertexOffsets[]{ 0 };
    for (auto& mesh : meshes) {
        if (mesh.GetInstanceCount() < 1)
            continue;
        m_pushConstants.meshIndex = meshIndex;
        meshIndex++;
        commandBuffer.pushConstants(
            shadowMapPipeline.m_pipelineLayout,
            vk::ShaderStageFlagBits::eVertex,
            0,
            sizeof(ShadowMapPushConstants),
            &m_pushConstants);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, shadowMapPipeline.m_pipelineLayout, 0, 1, &shadowMapPipeline.m_descriptorSets[0], 0, nullptr);
        for (auto& part : mesh.GetMeshParts()) {
            commandBuffer.bindVertexBuffers(0, 1, &mesh.vertexBuffers[part.bufferIndex]->Get().buffer, vertexOffsets);
            commandBuffer.bindIndexBuffer(mesh.indexBuffers[part.bufferIndex]->Get().buffer, 0, vk::IndexType::eUint32);
            commandBuffer.drawIndexed(mesh.GetIndicesCount(part.bufferIndex), mesh.GetInstanceCount(), 0, 0, 0);
        }
    }

    commandBuffer.endRenderPass();
    commandBuffer.end();

    m_submitInfo = vk::SubmitInfo(0, nullptr, nullptr, 1, &commandBuffer);
}
