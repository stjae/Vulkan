#include "shadowMap.h"

ShadowMap::ShadowMap()
{
    vkn::BufferInfo bufferInfo = { sizeof(glm::mat4), sizeof(glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_viewProjBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    shadowMapPipeline.UpdateViewProjBuffer(m_viewProjBuffer->Get().descriptorBufferInfo);
    // meshRenderPipeline.UpdateShadowMapViewProj(m_viewProjBuffer->Get().descriptorBufferInfo);
}

void ShadowMap::CreateShadowMap(vk::CommandBuffer& commandBuffer)
{
    CreateImage({ SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1 }, shadowMapDepthFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal, vkn::Image::s_clampSampler);

    vkn::Command::Begin(commandBuffer);
    vkn::Command::ChangeImageLayout(commandBuffer,
                                    m_bundle.image,
                                    {},
                                    vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                    { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });
    commandBuffer.end();
    vkn::Command::SubmitAndWait(commandBuffer);

    m_imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

    CreateImageView();
    CreateFramebuffer(shadowMapPipeline);
}

void ShadowMap::DrawShadowMap(vk::CommandBuffer& commandBuffer, std::vector<std::shared_ptr<Mesh>>& meshes)
{
    vkn::Command::ChangeImageLayout(commandBuffer,
                                    m_bundle.image,
                                    vk::ImageLayout::eUndefined,
                                    vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                    { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });

    vk::Viewport viewport({}, {}, (float)SHADOW_MAP_SIZE, (float)SHADOW_MAP_SIZE, 0.0f, 1.0f);
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor({ 0, 0 }, { SHADOW_MAP_SIZE, SHADOW_MAP_SIZE });
    commandBuffer.setScissor(0, 1, &scissor);

    vk::ClearValue clearValue;
    clearValue.depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

    vk::RenderPassBeginInfo renderPassBI(shadowMapPipeline.m_renderPass, m_framebuffer, { { 0, 0 }, { SHADOW_MAP_SIZE, SHADOW_MAP_SIZE } }, 1, &clearValue);
    commandBuffer.beginRenderPass(&renderPassBI, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, shadowMapPipeline.m_pipeline);
    int meshIndex = 0;
    vk::DeviceSize vertexOffsets[]{ 0 };
    for (auto& mesh : meshes) {
        if (mesh->GetInstanceCount() < 1)
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
        for (auto& part : mesh->GetMeshParts()) {
            commandBuffer.bindVertexBuffers(0, 1, &mesh->m_vertexBuffers[part.bufferIndex]->Get().buffer, vertexOffsets);
            commandBuffer.bindIndexBuffer(mesh->m_indexBuffers[part.bufferIndex]->Get().buffer, 0, vk::IndexType::eUint32);
            commandBuffer.drawIndexed(mesh->GetIndicesCount(part.bufferIndex), mesh->GetInstanceCount(), 0, 0, 0);
        }
    }
    commandBuffer.endRenderPass();

    vkn::Command::ChangeImageLayout(commandBuffer,
                                    m_bundle.image,
                                    vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                    vk::ImageLayout::eShaderReadOnlyOptimal,
                                    { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });
}

void ShadowMap::Update(const glm::vec3& lightPos)
{
    glm::mat4 lightProjection = glm::ortho(-1.0f * m_size, m_size, -1.0f * m_size, m_size, m_nearPlane, m_farPlane);
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 0.0f, 1.0f));
    m_viewProj = lightProjection * lightView;
    m_viewProjBuffer->Copy(&m_viewProj);
}
