#include "cascadedShadowMap.h"
#include "light.h"

void Cascade::Create(int index, const vkn::Image& depthImage)
{
    m_imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    m_imageViewCreateInfo.viewType = vk::ImageViewType::e2DArray;
    m_imageViewCreateInfo.subresourceRange.baseArrayLayer = index;
    m_imageViewCreateInfo.image = depthImage.Get().image;
    m_imageViewCreateInfo.format = depthImage.m_imageCreateInfo.format;
    CreateImageView();
    CreateFramebuffer(shadowMapPipeline, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
}

void Cascade::Draw(int index, vkn::Image& depthImage, std::vector<std::shared_ptr<Mesh>>& meshes, const vk::CommandBuffer& commandBuffer)
{
    vkn::Command::ChangeImageLayout(commandBuffer,
                                    depthImage.Get().image,
                                    vk::ImageLayout::eUndefined,
                                    vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                    { vk::ImageAspectFlagBits::eDepth, 0, 1, (uint32_t)index, 1 });

    vk::Viewport viewport({}, {}, (float)SHADOW_MAP_SIZE, (float)SHADOW_MAP_SIZE, 0.0f, 1.0f);
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor({ 0, 0 }, { SHADOW_MAP_SIZE, SHADOW_MAP_SIZE });
    commandBuffer.setScissor(0, 1, &scissor);

    vk::ClearValue clearValue;
    clearValue.depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

    vk::RenderPassBeginInfo renderPassBI(shadowMapPipeline.m_renderPass, m_framebuffer, { { 0, 0 }, { SHADOW_MAP_SIZE, SHADOW_MAP_SIZE } }, 1, &clearValue);
    commandBuffer.beginRenderPass(&renderPassBI, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, shadowMapPipeline.m_pipeline);
    m_pushConstants.viewProj = m_viewProj;
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
                                    depthImage.Get().image,
                                    vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                    vk::ImageLayout::eShaderReadOnlyOptimal,
                                    { vk::ImageAspectFlagBits::eDepth, 0, 1, (uint32_t)index, 1 });
}

CascadedShadowMap::CascadedShadowMap()
{
    vkn::BufferInfo bufferInfo = { sizeof(CascadeUBO), sizeof(CascadeUBO), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_UBOStagingBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    bufferInfo = { sizeof(CascadeUBO), sizeof(CascadeUBO), vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal };
    m_UBOBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    meshRenderPipeline.UpdateCascadeUBO(m_UBOBuffer->Get().descriptorBufferInfo);
}

void CascadedShadowMap::Create()
{
    m_depthImage.m_imageCreateInfo.arrayLayers = SHADOW_MAP_CASCADE_COUNT;
    m_depthImage.CreateImage({ SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1 }, shadowMapDepthFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    m_depthImage.m_imageViewCreateInfo.viewType = vk::ImageViewType::e2DArray;
    m_depthImage.m_imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    m_depthImage.m_imageViewCreateInfo.subresourceRange.layerCount = SHADOW_MAP_CASCADE_COUNT;
    m_depthImage.CreateImageView();

    for (int i = 0; i < m_cascades.size(); i++) {
        m_cascades[i].Create(i, m_depthImage);
    }

    vk::DescriptorImageInfo shadowMapImageInfo = { vkn::Image::s_clampSampler, m_depthImage.Get().imageView, vk::ImageLayout::eShaderReadOnlyOptimal };
    meshRenderPipeline.UpdateShadowMap(shadowMapImageInfo);
}

void CascadedShadowMap::UpdateCascades(Camera* camera, const DirLight& dirLight)
{
    float lastDepth = 0.0f;
    for (int i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {

        glm::vec3 frustumCorners[8] = {
            glm::vec3(-1.0f, 1.0f, 0.0f),
            glm::vec3(1.0f, 1.0f, 0.0f),
            glm::vec3(1.0f, -1.0f, 0.0f),
            glm::vec3(-1.0f, -1.0f, 0.0f),
            glm::vec3(-1.0f, 1.0f, 1.0f),
            glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(1.0f, -1.0f, 1.0f),
            glm::vec3(-1.0f, -1.0f, 1.0f),
        };

        float cascadeDepth = camera->GetCascadeDepth(i);
        glm::mat4 invCam = glm::inverse(camera->GetCascadeProj(i) * camera->GetUBO().view);
        for (int j = 0; j < 8; j++) {
            glm::vec4 worldCorner = invCam * glm::vec4(frustumCorners[j], 1.0f);
            frustumCorners[j] = worldCorner / worldCorner.w;
        }

        glm::vec3 depth = glm::vec3(cascadeDepth);
        for (int j = 0; j < 4; j++) {
            depth.y *= -1.0f;
            depth.x *= -1.0f;
            frustumCorners[j + 4] = frustumCorners[j] + depth;
            frustumCorners[j] = frustumCorners[j] + lastDepth;
        }

        glm::vec3 frustumCenter = glm::vec3(0.0f);
        for (int j = 0; j < 8; j++) {
            frustumCenter += frustumCorners[j];
        }
        frustumCenter /= 8.0f;

        float radius = 0.0f;
        for (int j = 0; j < 8; j++) {
            float distance = glm::length(frustumCorners[j] - frustumCenter);
            radius = glm::max(radius, distance);
        }

        glm::vec3 lightDir = normalize(-dirLight.pos);
        glm::mat4 lightView = glm::lookAt(frustumCenter - lightDir * radius, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightProj = glm::ortho(-radius, radius, -radius, radius, -radius, 2.0f * radius);

        m_cascades[i].m_depth = cascadeDepth * -1.0f;
        m_cascades[i].m_viewProj = lightProj * lightView;
        m_cascades[i].m_invProj = glm::inverse(lightProj);
        glm::vec4 left(-1.0f, -1.0f, 0.0f, 1.0f);
        glm::vec4 right(1.0f, 1.0f, 0.0f, 1.0f);
        left = m_cascades[i].m_invProj * left;
        right = m_cascades[i].m_invProj * right;
        left /= left.w;
        right /= right.w;
        m_cascades[i].m_frustumWidth = right.x - left.x;

        lastDepth = cascadeDepth;
    }
}

void CascadedShadowMap::UpdateUBO(const DirLight& dirLight, const vk::CommandBuffer& commandBuffer)
{
    for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
        m_UBO.depth[i] = m_cascades[i].m_depth;
        m_UBO.viewProj[i] = m_cascades[i].m_viewProj;
        m_UBO.invProj[i] = m_cascades[i].m_invProj;
        m_UBO.frustumWidth[i] = m_cascades[i].m_frustumWidth;
    }
    m_UBO.lightDir = normalize(-dirLight.pos);
    m_UBO.color = dirLight.color;
    m_UBO.intensity = dirLight.intensity;
    m_UBOStagingBuffer->Copy(&m_UBO);
    vkn::Command::CopyBufferToBuffer(commandBuffer, m_UBOStagingBuffer->Get().buffer, m_UBOBuffer->Get().buffer, m_UBOStagingBuffer->Get().bufferInfo.size);
}

void CascadedShadowMap::Draw(std::vector<std::shared_ptr<Mesh>>& meshes, const vk::CommandBuffer& commandBuffer)
{
    for (int i = 0; i < m_cascades.size(); i++) {
        m_cascades[i].Draw(i, m_depthImage, meshes, commandBuffer);
    }
}
