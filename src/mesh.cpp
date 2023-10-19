#include "mesh.h"

Mesh::Mesh()
{
    std::vector<float> pos = { 0.5f, -0.5f, 0.0f,
                               -0.5f, 0.5f, 0.0f,
                               -0.5f, -0.5f, 0.0f };

    std::vector<float> color = { 1.0f, 0.0f, 0.0f,
                                 0.0f, 1.0f, 0.0f,
                                 0.0f, 0.0f, 1.0f };

    std::vector<float> vertices;
    vertices.reserve(pos.size() + color.size());

    for (int i = 0; i < pos.size(); i += 3) {

        for (int j = i; j < i + 3; ++j) {
            vertices.push_back(pos[j]);
        }

        for (int j = i; j < i + 3; ++j) {
            vertices.push_back(color[j]);
        }
    }

    BufferInput input;
    input.size = sizeof(float) * vertices.size();
    input.usage = vk::BufferUsageFlagBits::eVertexBuffer;

    vertexBuffer = CreateBuffer(input);

    void* memoryLocation = device.mapMemory(vertexBuffer.bufferMemory, 0, input.size);
    memcpy(memoryLocation, vertices.data(), input.size);
    device.unmapMemory(vertexBuffer.bufferMemory);
}

Mesh::~Mesh()
{
    device.waitIdle();
    device.destroyBuffer(vertexBuffer.buffer);
    device.freeMemory(vertexBuffer.bufferMemory);
}