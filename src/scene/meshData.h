#ifndef MESHDATA_H
#define MESHDATA_H

#include "../API/buffer.h"

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

class MeshData
{
protected:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

public:
    static vk::VertexInputBindingDescription GetBindingDesc();
    static std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescs();
    void CreateVertexBuffer();
    void CreateIndexBuffer();

    size_t GetIndexCount() const { return indices.size(); }

    std::unique_ptr<Buffer> vertexStagingBuffer;
    std::unique_ptr<Buffer> vertexBuffer;

    std::unique_ptr<Buffer> indexStagingBuffer;
    std::unique_ptr<Buffer> indexBuffer;
};

#endif