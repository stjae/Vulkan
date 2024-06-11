#include "prefilteredCubemap.h"

void PrefilteredCubemap::DrawMipmap(const vk::CommandBuffer& commandBuffer, const Mesh& envCube, const vkn::Pipeline& cubemapPipeline)
{
    for (int mipLevel = 0; mipLevel < m_numMips; mipLevel++) {
        int mipSize = m_baseMipSize * std::pow(0.5, mipLevel);
        float roughness = (float)mipLevel / (float)(m_numMips - 1);
        DrawPrefilteredCubemap(mipSize, roughness, envCube, cubemapPipeline, commandBuffer);
        for (int i = 0; i < 6; i++) {
            m_imageViewCreateInfo.subresourceRange.baseArrayLayer = i;
            vkn::Command::ChangeImageLayout(commandBuffer, m_bundle.image, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal, m_imageViewCreateInfo.subresourceRange);
        }
        CopyToMipmap(mipSize, mipLevel, commandBuffer);
        for (int i = 0; i < 6; i++) {
            m_imageViewCreateInfo.subresourceRange.baseArrayLayer = i;
            vkn::Command::ChangeImageLayout(commandBuffer, m_bundle.image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eColorAttachmentOptimal, m_imageViewCreateInfo.subresourceRange);
        }
    }
    vkn::Command::ChangeImageLayout(commandBuffer, m_mipmap.Get().image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, m_mipmap.m_imageViewCreateInfo.subresourceRange);
    m_mipmapDescriptorImageInfo.imageView = m_mipmap.Get().imageView;
    m_mipmapDescriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    m_mipmapDescriptorImageInfo.sampler = m_mipmapSampler;
}

void PrefilteredCubemap::DrawPrefilteredCubemap(uint32_t mipSize, float roughness, const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, const vk::CommandBuffer& commandBuffer)
{
    vk::Viewport viewport({}, {}, (float)mipSize, (float)mipSize, 0.0f, 1.0f);
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor({ 0, 0 }, { m_imageSize, m_imageSize });
    commandBuffer.setScissor(0, 1, &scissor);

    for (uint32_t face = 0; face < 6; face++) {
        DrawPrefilteredCubemapFace(roughness, face, envCube, cubemapPipeline, commandBuffer);
    }
}

void PrefilteredCubemap::DrawPrefilteredCubemapFace(float roughness, uint32_t faceIndex, const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, const vk::CommandBuffer& commandBuffer)
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
    m_pushConstants.roughness = roughness;
    commandBuffer.pushConstants(
        cubemapPipeline.m_pipelineLayout,
        vk::ShaderStageFlagBits::eVertex,
        0,
        sizeof(PrefilteredCubemapPushConstants),
        &m_pushConstants);
    commandBuffer.bindVertexBuffers(0, 1, &envCube.m_vertexBuffers[0]->Get().buffer, vertexOffsets);
    commandBuffer.bindIndexBuffer(envCube.m_indexBuffers[0]->Get().buffer, 0, vk::IndexType::eUint32);
    commandBuffer.drawIndexed(envCube.GetIndicesCount(0), envCube.GetInstanceCount(), 0, 0, 0);

    commandBuffer.endRenderPass();
}