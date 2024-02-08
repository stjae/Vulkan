#ifndef MESH_H
#define MESH_H

#include "meshData.h"

class Mesh : public MeshData
{
    friend class Scene;

    bool isSelected_;
    std::string name_;
    glm::vec3 position_;
    glm::vec3 rotation_;
    TypeEnum::Mesh meshType_{};

    void CreateBuffers();
    void CreateSquare(const char* texturePath);
    void CreateCube(const char* texturePath);
    void LoadModel(const std::string& modelPath, const char* texturePath);
};

#endif