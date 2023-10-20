#include "mesh.h"

Mesh::Mesh()
{
    std::vector<float> pos = { 0.5f, -0.5f, 0.0f,
                               -0.5f, 0.5f, 0.0f,
                               -0.5f, -0.5f, 0.0f };

    std::vector<float> color = { 1.0f, 0.0f, 0.0f,
                                 0.0f, 1.0f, 0.0f,
                                 0.0f, 0.0f, 1.0f };

    vertices.reserve(pos.size() + color.size());

    for (int i = 0; i < pos.size(); i += 3) {

        for (int j = i; j < i + 3; ++j) {
            vertices.push_back(pos[j]);
        }

        for (int j = i; j < i + 3; ++j) {
            vertices.push_back(color[j]);
        }
    }
}

Mesh::~Mesh()
{
    device.waitIdle();

    device.destroyBuffer(m_stagingBuffer.buffer);
    device.freeMemory(m_stagingBuffer.memory);
    device.destroyBuffer(m_vertexBuffer.buffer);
    device.freeMemory(m_vertexBuffer.memory);
}