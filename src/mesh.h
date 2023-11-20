#ifndef _MESH_H_
#define _MESH_H_

#include "meshData.h"

struct MeshPushConstant {
    int index;
};

class Mesh : public MeshData
{
public:
    void CreateSquare();
    void CreateCube(const char* texturePath);
    void LoadModel(const char* modelPath, const char* texturePath);

    glm::vec3 pos;
    glm::vec3 rotation;

    MeshPushConstant pushConstant;
};

#endif