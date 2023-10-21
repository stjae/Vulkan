#include "scene.h"

Scene::Scene()
{
    m_positions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

    std::shared_ptr<Mesh> triangle = std::make_shared<Mesh>();
    m_meshes.push_back(triangle);

    for (auto mesh : m_meshes) {

        CreateVertexBuffer(mesh);
        CreateIndexBuffer(mesh);
    }
}

void Scene::CreateVertexBuffer(std::shared_ptr<Mesh> mesh)
{
    BufferInput stagingBufferInput;
    stagingBufferInput.size = sizeof(mesh->m_vertices[0]) * mesh->m_vertices.size();
    stagingBufferInput.usage = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBufferInput.properties = vk::MemoryPropertyFlagBits::eHostVisible |
                                    vk::MemoryPropertyFlagBits::eHostCoherent;

    mesh->m_vertexStagingBuffer = CreateBuffer(stagingBufferInput);

    void* memoryLocation = device.mapMemory(mesh->m_vertexStagingBuffer.memory, 0, stagingBufferInput.size);
    memcpy(memoryLocation, mesh->m_vertices.data(), stagingBufferInput.size);
    device.unmapMemory(mesh->m_vertexStagingBuffer.memory);

    BufferInput vertexBufferInput;
    vertexBufferInput.size = stagingBufferInput.size;
    vertexBufferInput.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    vertexBufferInput.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;

    mesh->m_vertexBuffer = CreateBuffer(vertexBufferInput);
}

void Scene::CreateIndexBuffer(std::shared_ptr<Mesh> mesh)
{
    BufferInput stagingBufferInput;
    stagingBufferInput.size = sizeof(mesh->m_indices[0]) * mesh->m_indices.size();
    stagingBufferInput.usage = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBufferInput.properties = vk::MemoryPropertyFlagBits::eHostVisible |
                                    vk::MemoryPropertyFlagBits::eHostCoherent;

    mesh->m_indexStagingBuffer = CreateBuffer(stagingBufferInput);

    void* memoryLocation = device.mapMemory(mesh->m_indexStagingBuffer.memory, 0, stagingBufferInput.size);
    memcpy(memoryLocation, mesh->m_indices.data(), stagingBufferInput.size);
    device.unmapMemory(mesh->m_indexStagingBuffer.memory);

    BufferInput indexBufferInput;
    indexBufferInput.size = stagingBufferInput.size;
    indexBufferInput.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    indexBufferInput.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;

    mesh->m_indexBuffer = CreateBuffer(indexBufferInput);
}
