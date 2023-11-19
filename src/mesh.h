#ifndef _MESH_H_
#define _MESH_H_

#include "meshData.h"

class Mesh : public MeshData
{
public:
    void CreateSquare();
    void CreateCube(const char* texturePath);
    void LoadModel(const char* modelPath, const char* texturePath);

    glm::vec3 pos;
    glm::vec3 rotation;
};

#endif