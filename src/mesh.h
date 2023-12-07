#ifndef _MESH_H_
#define _MESH_H_

#include "meshData.h"

struct UBO
{
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view;
    glm::mat4 proj;

    glm::vec3 eye;
};

struct MeshPushConstant
{
    int index;
};

class Mesh : public MeshData
{
    friend class MyImGui;
    friend class Scene;

    std::string name;
    bool isSelected;
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