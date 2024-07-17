#include "light.h"

void DirLight::Init()
{
    pos = { 0.1, 10.0f, 0.1f };
    intensity = 1.0f;
    color = { 1.0f, 1.0f, 1.0f };
}

void PointLight::Duplicate(int index)
{
    Add();
    m_UBOs.back() = m_UBOs[index];
}

void PointLight::Create()
{
    vkn::BufferInfo bufferInfo = { sizeof(PointLightUBO), sizeof(PointLightUBO), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_UBOStagingBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    bufferInfo = { sizeof(PointLightUBO), sizeof(PointLightUBO), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal };
    m_UBOBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    meshRenderPipeline.UpdatePointLightUBO(m_UBOBuffer->Get().descriptorBufferInfo);
    shadowCubemapPipeline.UpdatePointLightUBO(m_UBOBuffer->Get().descriptorBufferInfo);
}

void PointLight::Update(const vk::CommandBuffer& commandBuffer)
{
    if (!m_UBOs.empty()) {
        m_UBOStagingBuffer->Copy(m_UBOs.data());
        vkn::Command::CopyBufferToBuffer(commandBuffer, m_UBOStagingBuffer->Get().buffer, m_UBOBuffer->Get().buffer, m_UBOStagingBuffer->Get().bufferInfo.size);
    }
}

void PointLight::UpdateBuffer()
{
    if (!m_UBOs.empty()) {
        vkn::BufferInfo bufferInfo = { sizeof(PointLightUBO) * m_UBOs.size(), vk::WholeSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        m_UBOStagingBuffer.reset();
        m_UBOBuffer.reset();
        m_UBOStagingBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
        bufferInfo = { sizeof(PointLightUBO) * m_UBOs.size(), vk::WholeSize, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal };
        m_UBOBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
        m_UBOStagingBuffer->Copy(m_UBOs.data());
        meshRenderPipeline.UpdatePointLightUBO(m_UBOBuffer->Get().descriptorBufferInfo);
        shadowCubemapPipeline.UpdatePointLightUBO(m_UBOBuffer->Get().descriptorBufferInfo);
    }
}
