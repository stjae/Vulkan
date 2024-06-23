#include "grid.h"

void Grid::CreateGrid(int gridWidth)
{
    m_linePoints.clear();
    m_lineIndices.clear();

    for (int i = 0; i <= gridWidth; i++) {
        for (int j = 0; j <= gridWidth; j++) {
            m_linePoints.emplace_back(glm::vec3(i - gridWidth / 2, 0.0f, j - gridWidth / 2), glm::vec3(0.5f));
        }
    }

    for (int i = 0; i <= gridWidth; i++) {
        for (int j = 0; j <= gridWidth; j++) {
            m_linePoints.emplace_back(glm::vec3(j - gridWidth / 2, 0.0f, i - gridWidth / 2), glm::vec3(0.5f));
        }
    }

    int sub = 0;
    for (int i = 0; i <= gridWidth * 2 + 1; i++) {
        for (int j = 0; j < gridWidth * 2; j++) {
            if (j % 2 == 0 && j != 0)
                sub++;
            m_lineIndices.push_back(m_lineIndices.size() - sub);
        }
    }
}

void Grid::CreateBuffer()
{
    vkn::BufferInfo bufferInfo = { sizeof(LinePoint) * m_linePoints.size(), sizeof(LinePoint), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_vertexStagingBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    m_vertexStagingBuffer->Copy(m_linePoints.data());

    bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    bufferInfo.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    m_vertexBuffer = std::make_unique<vkn::Buffer>(bufferInfo);

    bufferInfo = { sizeof(uint32_t) * m_lineIndices.size(), sizeof(uint32_t), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_indexStagingBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    m_indexStagingBuffer->Copy(m_lineIndices.data());

    bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    bufferInfo.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    m_indexBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
}

void Grid::CopyBuffer(const vk::CommandBuffer& commandBuffer) const
{
    vkn::Command::CopyBufferToBuffer(commandBuffer, m_vertexStagingBuffer->Get().buffer, m_vertexBuffer->Get().buffer, m_vertexStagingBuffer->Get().bufferInfo.size);
    vkn::Command::CopyBufferToBuffer(commandBuffer, m_indexStagingBuffer->Get().buffer, m_indexBuffer->Get().buffer, m_indexStagingBuffer->Get().bufferInfo.size);
}
