#ifndef MESH_H
#define MESH_H

#include "meshData.h"

class Mesh : public MeshData
{
    friend class Scene;

    std::string name_;
    glm::vec3 position_;
    glm::vec3 rotation_;
    TypeEnum::Mesh meshType_{};

    void CreateBuffers();
    void CreateSquare(glm::vec3 color = glm::vec3(0.5f), const char* texturePath = nullptr);
    void CreateCube(glm::vec3 color = glm::vec3(0.5f), const char* texturePath = nullptr);
    void CreateSphere(float scale = 1.0f, glm::vec3 color = glm::vec3(0.5f), const char* name = nullptr, const char* texturePath = nullptr);
    void LoadModel(const std::string& modelPath, const char* texturePath = nullptr, glm::vec3 color = glm::vec3(0.5f));
};

#endif