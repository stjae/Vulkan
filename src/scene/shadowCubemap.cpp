#include "shadowCubemap.h"

void ShadowCubemap::CreateProjBuffer()
{
    vkn::BufferInfo bufferInfo = { sizeof(glm::mat4), sizeof(glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    s_shadowCubemapProjBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    s_shadowCubemapProj = glm::perspective(glm::radians(90.0f), 1.0f, s_zNear, s_zFar);
    s_shadowCubemapProjBuffer->Copy(&s_shadowCubemapProj);
    shadowCubemapPipeline.UpdateProjBuffer(s_shadowCubemapProjBuffer->Get().descriptorBufferInfo);
}

void ShadowCubemap::CreateShadowMap(const vk::CommandBuffer& commandBuffer)
{
    CreateCubemap(shadowCubemapSize, shadowMapImageFormat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, commandBuffer);
    vkn::Cubemap::ChangeImageLayout(commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal);
    CreateDepthImage(commandBuffer);
    CreateFramebuffer();
}

void ShadowCubemap::CreateDepthImage(const vk::CommandBuffer& commandBuffer)
{
    m_depthImage.CreateImage({ shadowCubemapSize, shadowCubemapSize, 1 }, shadowMapDepthFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    vkn::Command::ChangeImageLayout(commandBuffer, m_depthImage.Get().image, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });
    m_depthImage.m_imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    m_depthImage.CreateImageView();
}

void ShadowCubemap::CreateFramebuffer()
{
    std::array<vk::ImageView, 2> attachments;
    attachments[1] = m_depthImage.Get().imageView;

    vk::FramebufferCreateInfo frameBufferCI({}, shadowCubemapPipeline.m_renderPass, attachments.size(), attachments.data(), shadowCubemapSize, shadowCubemapSize, 1);

    for (uint32_t i = 0; i < 6; i++) {
        attachments[0] = m_cubemapFaceImageViews[i];
        vkn::CheckResult(vkn::Device::Get().device.createFramebuffer(&frameBufferCI, nullptr, &m_framebuffers[i]));
    }
}

void ShadowCubemap::DrawShadowMap(int lightIndex, PointLight& light, std::vector<std::shared_ptr<Mesh>>& meshes, const vk::CommandBuffer& commandBuffer)
{
    vk::Viewport viewport({}, {}, (float)shadowCubemapSize, (float)shadowCubemapSize, 0.0f, 1.0f);
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor({ 0, 0 }, { shadowCubemapSize, shadowCubemapSize });
    commandBuffer.setScissor(0, 1, &scissor);

    for (uint32_t face = 0; face < 6; face++) {
        UpdateCubemapFace(face, lightIndex, light, meshes, commandBuffer);
    }
}

void ShadowCubemap::UpdateCubemapFace(uint32_t faceIndex, int lightIndex, PointLight& light, std::vector<std::shared_ptr<Mesh>>& meshes, const vk::CommandBuffer& commandBuffer)
{
    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0] = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

    vk::RenderPassBeginInfo renderPassBI(shadowCubemapPipeline.m_renderPass, m_framebuffers[faceIndex], { { 0, 0 }, { shadowCubemapSize, shadowCubemapSize } }, 2, clearValues.data());

    glm::mat4 viewMatrix;
    glm::vec3 lightPos = light.Get()[lightIndex].model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    switch (faceIndex) {
    case 0: // POS X
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    case 1: // NEG X
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    case 2: // POS Y
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        break;
    case 3: // NEG Y
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        break;
    case 4: // POS Z
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    case 5: // NEG Z
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    }

    m_pushConstants.view = viewMatrix;
    m_pushConstants.lightIndex = lightIndex;

    commandBuffer.beginRenderPass(&renderPassBI, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, shadowCubemapPipeline.m_pipeline);
    int meshIndex = 0;
    vk::DeviceSize vertexOffsets[]{ 0 };
    for (auto& mesh : meshes) {
        if (mesh->GetInstanceCount() < 1)
            continue;
        m_pushConstants.meshIndex = meshIndex;
        meshIndex++;
        commandBuffer.pushConstants(
            shadowCubemapPipeline.m_pipelineLayout,
            vk::ShaderStageFlagBits::eVertex,
            0,
            sizeof(ShadowCubemapPushConstants),
            &m_pushConstants);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, shadowCubemapPipeline.m_pipelineLayout, 0, 1, &shadowCubemapPipeline.m_descriptorSets[0], 0, nullptr);
        for (auto& part : mesh->GetMeshParts()) {
            commandBuffer.bindVertexBuffers(0, 1, &mesh->m_vertexBuffers[part.bufferIndex]->Get().buffer, vertexOffsets);
            commandBuffer.bindIndexBuffer(mesh->m_indexBuffers[part.bufferIndex]->Get().buffer, 0, vk::IndexType::eUint32);
            commandBuffer.drawIndexed(mesh->GetIndicesCount(part.bufferIndex), mesh->GetInstanceCount(), 0, 0, 0);
        }
    }
    commandBuffer.endRenderPass();
}

void ShadowCubemap::DestroyBuffer()
{
    s_shadowCubemapProjBuffer.reset();
}
