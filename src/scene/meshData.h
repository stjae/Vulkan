#ifndef MESHDATA_H
#define MESHDATA_H

#include "../vulkan/buffer.h"

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texcoord;
    glm::vec3 tangent;

    Vertex()
        : pos(0.0f), normal(0.0f), color(0.0f), texcoord(0.0f), tangent(0.0f) {}
    Vertex(glm::vec3& pos, glm::vec3&& normal, glm::vec3& color, glm::vec2& texcoord, glm::vec3& tangent)
        : pos(pos), normal(normal), color(color), texcoord(texcoord), tangent(tangent) {}
    Vertex(glm::vec3& pos, glm::vec3& normal, glm::vec3& color, glm::vec2& texcoord, glm::vec3& tangent)
        : pos(pos), normal(normal), color(color), texcoord(texcoord), tangent(tangent) {}
};

class MeshData
{
protected:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

public:
    static vk::VertexInputBindingDescription GetBindingDesc();
    static std::array<vk::VertexInputAttributeDescription, 5> GetAttributeDescs();
    void CreateVertexBuffer();
    void CreateIndexBuffer();

    size_t GetIndexCount() const { return indices.size(); }
    size_t GetVertexCount() const { return vertices.size(); }

    std::unique_ptr<Buffer> vertexStagingBuffer;
    std::unique_ptr<Buffer> vertexBuffer;

    std::unique_ptr<Buffer> indexStagingBuffer;
    std::unique_ptr<Buffer> indexBuffer;
};

#endif