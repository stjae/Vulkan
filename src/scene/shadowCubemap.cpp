#include "shadowCubemap.h"

void ShadowCubemap::CreateProjBuffer()
{
    vkn::BufferInfo bufferInfo = { sizeof(glm::mat4), sizeof(glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    s_shadowCubemapProjBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    s_shadowCubemapProj = glm::perspective(glm::radians(90.0f), 1.0f, s_zNear, s_zFar);
    s_shadowCubemapProjBuffer->Copy(&s_shadowCubemapProj);
    shadowCubemapPipeline.UpdateProjBuffer(s_shadowCubemapProjBuffer->Get().descriptorBufferInfo);
}

void ShadowCubemap::CreateShadowMap()
{
    vkn::Command::CreateCommandPool(m_commandPool);
    vkn::Command::AllocateCommandBuffer(m_commandPool, m_commandBuffers);

    CreateCubemap(shadowCubemapSize, shadowMapImageFormat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);

    CreateDepthImage();
    CreateFramebuffer();
}

void ShadowCubemap::CreateDepthImage()
{
    m_depthImage.CreateImage({ shadowCubemapSize, shadowCubemapSize, 1 }, shadowMapDepthFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);

    // TODO: Use subpass dependency
    vkn::Command::Begin(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    vkn::Command::SetImageMemoryBarrier(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()],
                                        m_depthImage.Get().image,
                                        {},
                                        vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                        {},
                                        vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });
    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].end();
    vkn::Command::SubmitAndWait(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);

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

void ShadowCubemap::DrawShadowMap(int lightIndex, PointLight& light, std::vector<std::shared_ptr<Mesh>>& meshes)
{
    vkn::Command::Begin(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()], vk::CommandBufferUsageFlagBits::eSimultaneousUse);

    vk::Viewport viewport({}, {}, (float)shadowCubemapSize, (float)shadowCubemapSize, 0.0f, 1.0f);
    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].setViewport(0, 1, &viewport);

    vk::Rect2D scissor({ 0, 0 }, { shadowCubemapSize, shadowCubemapSize });
    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].setScissor(0, 1, &scissor);

    for (uint32_t face = 0; face < 6; face++) {
        UpdateCubemapFace(face, lightIndex, light, meshes);
    }

    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].end();
    vkn::Device::s_submitInfos.emplace_back(0, nullptr, nullptr, 1, &m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
}

void ShadowCubemap::UpdateCubemapFace(uint32_t faceIndex, int lightIndex, PointLight& light, std::vector<std::shared_ptr<Mesh>>& meshes)
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

    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].beginRenderPass(&renderPassBI, vk::SubpassContents::eInline);

    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindPipeline(vk::PipelineBindPoint::eGraphics, shadowCubemapPipeline.m_pipeline);
    int meshIndex = 0;
    vk::DeviceSize vertexOffsets[]{ 0 };
    for (auto& mesh : meshes) {
        if (mesh->GetInstanceCount() < 1)
            continue;
        m_pushConstants.meshIndex = meshIndex;
        meshIndex++;
        m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].pushConstants(
            shadowCubemapPipeline.m_pipelineLayout,
            vk::ShaderStageFlagBits::eVertex,
            0,
            sizeof(ShadowCubemapPushConstants),
            &m_pushConstants);
        m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, shadowCubemapPipeline.m_pipelineLayout, 0, 1, &shadowCubemapPipeline.m_descriptorSets[0], 0, nullptr);
        for (auto& part : mesh->GetMeshParts()) {
            m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindVertexBuffers(0, 1, &mesh->m_vertexBuffers[part.bufferIndex]->Get().buffer, vertexOffsets);
            m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindIndexBuffer(mesh->m_indexBuffers[part.bufferIndex]->Get().buffer, 0, vk::IndexType::eUint32);
            m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].drawIndexed(mesh->GetIndicesCount(part.bufferIndex), mesh->GetInstanceCount(), 0, 0, 0);
        }
    }

    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].endRenderPass();
}

ShadowCubemap::~ShadowCubemap()
{
    vkn::Device::Get().device.destroy(m_commandPool);
}

void ShadowCubemap::DestroyBuffer()
{
    s_shadowCubemapProjBuffer.reset();
}
