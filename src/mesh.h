#ifndef _MESH_H_
#define _MESH_H_

#include "meshData.h"

class Mesh : public MeshData
{
    friend class Scene;
    friend class MyImGui;

    bool isSelected;
    std::string name;
    glm::vec3 pos;
    glm::vec3 rotation;

    void CreateSquare();
    void CreateCube(const char* texturePath);
    void LoadModel(const char* modelPath, const char* texturePath);

    void CreateBuffers();
};

#endif