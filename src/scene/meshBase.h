#ifndef MESHDATA_H
#define MESHDATA_H

#include "../vulkan/buffer.h"
#include <btBulletCollisionCommon.h>

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texcoord;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    Vertex()
        : pos(0.0f), normal(0.0f), texcoord(0.0f), tangent(0.0f), bitangent(0.0f) {}
    Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 texcoord, glm::vec3 tangent = glm::vec3(0.0f), glm::vec3 bitangent = glm::vec3(0.0f))
        : pos(pos), normal(normal), texcoord(texcoord), tangent(tangent), bitangent(bitangent) {}
};

class MeshBase
{
protected:
    std::vector<std::vector<Vertex>> m_vertexContainers;
    std::vector<std::vector<uint32_t>> m_indexContainers;
    // for bullet physics debugger
    std::vector<std::vector<float>> m_posContainers;

    // Physics
    std::vector<btIndexedMesh> m_bulletMeshes;
    btTriangleIndexVertexArray m_bulletVertexArray;

public:
    static vk::VertexInputBindingDescription GetBindingDesc();
    static std::array<vk::VertexInputAttributeDescription, 5> GetAttributeDescs();
    void CreateVertexBuffers(std::vector<Vertex>& vertices);
    void CreateIndexBuffers(std::vector<uint32_t>& indices);

    size_t GetIndicesCount(uint32_t bufferIndex) const { return m_indexContainers[bufferIndex].size(); }

    std::vector<std::unique_ptr<vkn::Buffer>> m_vertexStagingBuffers;
    std::vector<std::unique_ptr<vkn::Buffer>> m_vertexBuffers;

    std::vector<std::unique_ptr<vkn::Buffer>> m_indexStagingBuffers;
    std::vector<std::unique_ptr<vkn::Buffer>> m_indexBuffers;
};

#endif