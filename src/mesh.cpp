#include "mesh.h"

Mesh::Mesh()
{
    std::vector<float> pos = { -0.5f, 0.5f, 0.0f,
                               0.5f, 0.5f, 0.0f,
                               0.5f, -0.5f, 0.0f,
                               -0.5f, -0.5f, 0.0f };

    std::vector<float> color = { 1.0f, 0.0f, 0.0f,
                                 0.0f, 1.0f, 0.0f,
                                 0.0f, 0.0f, 1.0f,
                                 1.0f, 1.0f, 1.0f };

    m_vertices.reserve(pos.size() + color.size());

    for (int i = 0; i < pos.size(); i += 3) {

        for (int j = i; j < i + 3; ++j) {
            m_vertices.push_back(pos[j]);
        }

        for (int j = i; j < i + 3; ++j) {
            m_vertices.push_back(color[j]);
        }
    }

    m_indices = { 0, 1, 2, 2, 3, 0 };
}

Mesh::~Mesh()
{
    device.waitIdle();

    device.destroyBuffer(m_vertexStagingBuffer.buffer);
    device.freeMemory(m_vertexStagingBuffer.memory);

    device.destroyBuffer(m_indexStagingBuffer.buffer);
    device.freeMemory(m_indexStagingBuffer.memory);

    device.destroyBuffer(m_vertexBuffer.buffer);
    device.freeMemory(m_vertexBuffer.memory);

    device.destroyBuffer(m_indexBuffer.buffer);
    device.freeMemory(m_indexBuffer.memory);
}