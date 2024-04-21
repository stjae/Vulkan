#include "shadowMap.h"

void ShadowMap::CreateFramebuffer(vk::CommandBuffer& commandBuffer)
{
    vk::ImageView attachment;
    attachment = imageBundle_.imageView;

    vk::FramebufferCreateInfo frameBufferCI({}, shadowMapPipeline.renderPass, 1, &attachment, shadowMapSize, shadowMapSize, 1);

    vkn::CheckResult(vkn::Device::GetBundle().device.createFramebuffer(&frameBufferCI, nullptr, &framebuffer_));
}

void ShadowMap::CreateShadowMap(vk::CommandBuffer& commandBuffer)
{
    CreateImage({ shadowMapSize, shadowMapSize, 1 }, shadowMapDepthFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal, vkn::Image::clampSampler);

    vkn::Command::Begin(commandBuffer);
    vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                        imageBundle_.image,
                                        {},
                                        vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                        {},
                                        vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });
    commandBuffer.end();
    vkn::Command::Submit(&commandBuffer, 1);

    imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    CreateImageView();

    CreateFramebuffer(commandBuffer);
}

void ShadowMap::DrawShadowMap(vk::CommandBuffer& commandBuffer, std::vector<MeshModel>& meshes)
{
    vkn::Command::Begin(commandBuffer);
    vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                        imageBundle_.image,
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

    vk::RenderPassBeginInfo renderPassBI(shadowMapPipeline.renderPass, framebuffer_, { { 0, 0 }, { shadowMapSize, shadowMapSize } }, 1, &clearValue);
    commandBuffer.beginRenderPass(&renderPassBI, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, shadowMapPipeline.pipeline);
    int meshIndex = 0;
    vk::DeviceSize vertexOffsets[]{ 0 };
    for (auto& mesh : meshes) {
        if (mesh.GetInstanceCount() < 1)
            continue;
        pushConstants_.meshIndex = meshIndex;
        meshIndex++;
        commandBuffer.pushConstants(
            shadowMapPipeline.pipelineLayout,
            vk::ShaderStageFlagBits::eVertex,
            0,
            sizeof(ShadowMapPushConstants),
            &pushConstants_);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, shadowMapPipeline.pipelineLayout, 0, 1, &shadowMapPipeline.descriptorSets[0], 0, nullptr);
        for (auto& part : mesh.GetMeshParts()) {
            commandBuffer.bindVertexBuffers(0, 1, &mesh.vertexBuffers[part.bufferIndex]->GetBundle().buffer, vertexOffsets);
            commandBuffer.bindIndexBuffer(mesh.indexBuffers[part.bufferIndex]->GetBundle().buffer, 0, vk::IndexType::eUint32);
            commandBuffer.drawIndexed(mesh.GetIndicesCount(part.bufferIndex), mesh.GetInstanceCount(), 0, 0, 0);
        }
    }

    commandBuffer.endRenderPass();
    commandBuffer.end();

    vkn::Command::Submit(&commandBuffer, 1);
}
