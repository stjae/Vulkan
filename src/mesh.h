#ifndef _MESH_H_
#define _MESH_H_

#include "common.h"
#include "API/memory.h"

class Mesh
{
public:
    Mesh();
    ~Mesh();
    Buffer vertexBuffer;
};

inline vk::VertexInputBindingDescription GetPosColorBindingDesc()
{
    vk::VertexInputBindingDescription bindingDesc;
    bindingDesc.setBinding(0);
    bindingDesc.setStride(3 * sizeof(float) + 3 * sizeof(float));
    bindingDesc.setInputRate(vk::VertexInputRate::eVertex);

    return bindingDesc;
}

inline std::array<vk::VertexInputAttributeDescription, 2> GetPosColorAttributeDescs()
{
    std::array<vk::VertexInputAttributeDescription, 2> attributes;

    // Pos
    attributes[0].setBinding(0);
    attributes[0].setLocation(0);
    attributes[0].setFormat(vk::Format::eR32G32B32Sfloat);
    attributes[0].setOffset(0);

    // Color
    attributes[1].setBinding(0);
    attributes[1].setLocation(1);
    attributes[1].setFormat(vk::Format::eR32G32B32Sfloat);
    attributes[1].setOffset(3 * sizeof(float));

    return attributes;
}

#endif