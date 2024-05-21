#include "shadowCubemap.h"

void ShadowCubemap::CreateShadowMap(vk::CommandBuffer& commandBuffer)
{
    CreateCubemap(shadowCubemapSize, shadowMapImageFormat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, commandBuffer);

    vkn::Command::Begin(commandBuffer);

    CreateDepthImage(commandBuffer);
    CreateFramebuffer();
}

void ShadowCubemap::CreateDepthImage(vk::CommandBuffer& commandBuffer)
{
    m_depthImage.CreateImage({ shadowCubemapSize, shadowCubemapSize, 1 }, shadowMapDepthFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);

    vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                        m_depthImage.Get().image,
                                        {},
                                        vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                        {},
                                        vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });
    commandBuffer.end();
    vkn::Command::SubmitAndWait(commandBuffer);

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

void ShadowCubemap::DrawShadowMap(int lightIndex, std::vector<PointLightUBO>& lights, std::vector<Mesh>& meshes)
{
    vkn::Command::Begin(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()], vk::CommandBufferUsageFlagBits::eSimultaneousUse);

    vk::Viewport viewport({}, {}, (float)shadowCubemapSize, (float)shadowCubemapSize, 0.0f, 1.0f);
    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].setViewport(0, 1, &viewport);

    vk::Rect2D scissor({ 0, 0 }, { shadowCubemapSize, shadowCubemapSize });
    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].setScissor(0, 1, &scissor);

    for (uint32_t face = 0; face < 6; face++) {
        UpdateCubemapFace(face, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()], lightIndex, lights, meshes);
    }

    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].end();
    vkn::Device::s_submitInfos.emplace_back(0, nullptr, nullptr, 1, &m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
}

void ShadowCubemap::UpdateCubemapFace(uint32_t faceIndex, vk::CommandBuffer& commandBuffer, int lightIndex, std::vector<PointLightUBO>& lights, std::vector<Mesh>& meshes)
{
    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0] = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

    vk::RenderPassBeginInfo renderPassBI(shadowCubemapPipeline.m_renderPass, m_framebuffers[faceIndex], { { 0, 0 }, { shadowCubemapSize, shadowCubemapSize } }, 2, clearValues.data());

    glm::mat4 viewMatrix = lights[lightIndex].model;
    glm::vec3 lightPos = lights[lightIndex].model * glm::vec4(lights[lightIndex].pos, 1.0f);
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
        if (mesh.GetInstanceCount() < 1)
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
        for (auto& part : mesh.GetMeshParts()) {
            commandBuffer.bindVertexBuffers(0, 1, &mesh.m_vertexBuffers[part.bufferIndex]->Get().buffer, vertexOffsets);
            commandBuffer.bindIndexBuffer(mesh.m_indexBuffers[part.bufferIndex]->Get().buffer, 0, vk::IndexType::eUint32);
            commandBuffer.drawIndexed(mesh.GetIndicesCount(part.bufferIndex), mesh.GetInstanceCount(), 0, 0, 0);
        }
    }

    commandBuffer.endRenderPass();
}