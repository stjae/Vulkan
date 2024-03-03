#ifndef MESH_H
#define MESH_H

#include "meshData.h"

struct MeshInstanceData
{
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 invTranspose = glm::mat4(1.0f);
    int32_t meshID = 0;
    int32_t textureID = 0;
    int useTexture = 0;
    float padding = 0.0f;
};

class Mesh : public MeshData
{
    friend class Scene;

    std::string name_;
    glm::vec3 position_;
    glm::vec3 rotation_;
    TypeEnum::Mesh meshType_{};

    void CreateBuffers();
    void CreateSquare(float scale = 1.0f, glm::vec3 color = glm::vec3(0.5f), const char* texturePath = nullptr);
    void CreateCube(float scale = 1.0f, glm::vec3 color = glm::vec3(0.5f), const char* texturePath = nullptr);
    void CreateSphere(float scale = 1.0f, glm::vec3 color = glm::vec3(0.5f), const char* name = nullptr, const char* texturePath = nullptr);
    void LoadModel(const std::string& modelPath, const char* texturePath = nullptr, glm::vec3 color = glm::vec3(0.5f));
};

#endif