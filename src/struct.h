#ifndef STRUCT_H
#define STRUCT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "enum.h"
#include <bullet/btBulletCollisionCommon.h>

struct MeshPart
{
    int32_t bufferIndex;
    int32_t materialID;

    MeshPart(int32_t bufferIndex, int32_t materialID) : bufferIndex(bufferIndex), materialID(materialID) {}
};

struct MeshInstancePhysicsInfo
{
    // model matrix before simulation
    glm::mat4 initialModel;
    eRigidBodyType rigidBodyType;
    eRigidBodyShape rigidBodyShape;
    btRigidBody* rigidBodyPtr;
    glm::mat4 matrix;
    glm::vec3 scale;
    glm::vec3 size = { 1.0f, 1.0f, 1.0f };
};

struct MeshInstanceUBO
{
    glm::mat4 model;
    glm::mat4 invTranspose;
    int32_t meshID;
    int32_t textureID;
    int32_t instanceID;
    int32_t useTexture;
    glm::vec3 albedo;
    float metallic;
    float roughness;

    std::unique_ptr<MeshInstancePhysicsInfo> pInfo;

    MeshInstanceUBO(int32_t meshID, int32_t instanceID, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f)) : model(1.0f), invTranspose(1.0f), meshID(meshID), textureID(0), useTexture(true), instanceID(instanceID), albedo({ 0.5, 0.5, 0.5 }), metallic(0.0f), roughness(1.0f)
    {
        model = glm::translate(model, pos);
        model = glm::scale(model, scale);
        invTranspose = model;
        invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        invTranspose = glm::transpose(glm::inverse(invTranspose));
    }
    MeshInstanceUBO(MeshInstanceUBO&& other)
    {
        *this = other;
        this->pInfo = std::move(other.pInfo);
    }
    MeshInstanceUBO& operator=(const MeshInstanceUBO& other)
    {
        if (this != &other) {
            this->model = other.model;
            this->invTranspose = other.invTranspose;
            this->meshID = other.meshID;
            this->textureID = other.textureID;
            this->instanceID = other.instanceID;
            this->useTexture = other.useTexture;
            this->albedo = other.albedo;
            this->metallic = other.metallic;
            this->roughness = other.roughness;
        }
        return *this;
    }
    // MeshInstanceUBO& operator=(MeshInstanceUBO&& other) {}
};

struct MaterialFilePath
{
    std::string albedo;
    std::string normal;
    std::string metallic;
    std::string roughness;
};

struct PointLightUBO
{
    glm::mat4 model;
    glm::vec3 pos;
    float padding0;
    glm::vec3 color;
    float padding1;

    PointLightUBO() : model(glm::mat4(1.0f)), pos(glm::vec3(0.0f)), color(1.0f), padding0(0.0f), padding1(0.0f) {}
    explicit PointLightUBO(glm::vec3&& pos) : model(glm::mat4(1.0f)), pos(pos), color(1.0f), padding0(0.0f), padding1(0.0f) {}
};

struct DirLightUBO
{
    glm::vec3 dir;
    float intensity = 1.0f;
    glm::vec3 color = glm::vec3(1.0f);
};

struct Resource
{
    std::string filePath;
    std::string fileName;
    void* ptr;

    explicit Resource(std::string& path) : ptr(nullptr)
    {
        this->filePath = path;
        this->fileName = path.substr(path.find_last_of("/\\") + 1, path.rfind('.') - path.find_last_of("/\\") - 1);
    }
};

#endif