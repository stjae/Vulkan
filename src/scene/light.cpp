#include "light.h"

DirLight::DirLight()
{
    vkn::BufferInfo bufferInfo = { sizeof(DirLightUBO), sizeof(DirLightUBO), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_dirLightUBOBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    // meshRenderPipeline.UpdateDirLightUBO(m_dirLightUBOBuffer->Get().descriptorBufferInfo);
}

void DirLight::Update()
{
    m_UBO.dir = glm::normalize(m_position);
    m_dirLightUBOBuffer->Copy(&m_UBO);
}

void PointLight::Duplicate(int index)
{
    Add();
    m_UBOs.back() = m_UBOs[index];
}

void PointLight::Create(const vk::CommandPool& commandPool)
{
    vkn::Command::AllocateCommandBuffer(commandPool, m_commandBuffers);
    vkn::BufferInfo bufferInfo = { sizeof(PointLightUBO), sizeof(PointLightUBO), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_UBOStagingBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    bufferInfo = { sizeof(PointLightUBO), sizeof(PointLightUBO), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal };
    m_UBOBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    meshRenderPipeline.UpdatePointLightUBO(m_UBOBuffer->Get().descriptorBufferInfo);
    shadowCubemapPipeline.UpdatePointLightUBO(m_UBOBuffer->Get().descriptorBufferInfo);
}

void PointLight::Update()
{
    if (!m_UBOs.empty()) {
        m_UBOStagingBuffer->Copy(m_UBOs.data());
        vkn::Command::Begin(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()], vk::CommandBufferUsageFlagBits::eSimultaneousUse);
        vkn::Command::CopyBufferToBuffer(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()], m_UBOStagingBuffer->Get().buffer, m_UBOBuffer->Get().buffer, m_UBOStagingBuffer->Get().bufferInfo.size);
        m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].end();
        vkn::Device::s_submitInfos.emplace_back(0, nullptr, nullptr, 1, &m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    }
}

void PointLight::UpdateBuffer()
{
    if (!m_UBOs.empty()) {
        vkn::Command::Begin(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()], vk::CommandBufferUsageFlagBits::eSimultaneousUse);
        vkn::BufferInfo bufferInfo = { sizeof(PointLightUBO) * m_UBOs.size(), vk::WholeSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        m_UBOStagingBuffer.reset();
        m_UBOBuffer.reset();
        m_UBOStagingBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
        bufferInfo = { sizeof(PointLightUBO) * m_UBOs.size(), vk::WholeSize, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal };
        m_UBOBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
        m_UBOStagingBuffer->Copy(m_UBOs.data());
        vkn::Command::CopyBufferToBuffer(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()], m_UBOStagingBuffer->Get().buffer, m_UBOBuffer->Get().buffer, m_UBOStagingBuffer->Get().bufferInfo.size);
        m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].end();
        vkn::Device::s_submitInfos.emplace_back(0, nullptr, nullptr, 1, &m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
        meshRenderPipeline.UpdatePointLightUBO(m_UBOBuffer->Get().descriptorBufferInfo);
        shadowCubemapPipeline.UpdatePointLightUBO(m_UBOBuffer->Get().descriptorBufferInfo);
    }
}
