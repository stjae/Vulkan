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
    Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec3 color, glm::vec2 texcoord, glm::vec3 tangent)
        : pos(pos), normal(normal), color(color), texcoord(texcoord), tangent(tangent) {}
};

class MeshBase
{
protected:
    std::vector<std::vector<Vertex>> vertexContainers_;
    std::vector<std::vector<uint32_t>> indexContainers_;

public:
    static vk::VertexInputBindingDescription GetBindingDesc();
    static std::array<vk::VertexInputAttributeDescription, 5> GetAttributeDescs();
    void CreateVertexBuffers(std::vector<Vertex>& vertices);
    void CreateIndexBuffers(std::vector<uint32_t>& indices);

    size_t GetIndicesCount(uint32_t bufferIndex) const { return indexContainers_[bufferIndex].size(); }

    std::vector<std::unique_ptr<vkn::Buffer>> vertexStagingBuffers;
    std::vector<std::unique_ptr<vkn::Buffer>> vertexBuffers;

    std::vector<std::unique_ptr<vkn::Buffer>> indexStagingBuffers;
    std::vector<std::unique_ptr<vkn::Buffer>> indexBuffers;
};

#endif