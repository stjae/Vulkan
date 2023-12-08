#ifndef _MESH_H_
#define _MESH_H_

#include "meshData.h"

struct UBO
{
    glm::mat4 model = glm::mat4(1.0f);
};

struct MeshPushConstant
{
    int index;
};

class Mesh : public MeshData
{
    friend class MyImGui;
    friend class Scene;

    bool isSelected;
    std::string name;
    glm::vec3 pos;
    glm::vec3 rotation;
    UBO ubo;
    MeshPushConstant pushConstant;
    std::unique_ptr<Buffer> matrixUniformBuffer;

    void CreateSquare();
    void CreateCube(const char* texturePath);
    void LoadModel(const char* modelPath, const char* texturePath);

public:
    const MeshPushConstant& GetPushConstant() { return pushConstant; }
};

#endif