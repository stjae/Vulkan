#ifndef STRUCT_H
#define STRUCT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "enum.h"
#include <bullet/btBulletCollisionCommon.h>
#include "vulkan/buffer.h"
#include "scene/physicsDebugDrawer.h"

struct MeshPart
{
    int32_t bufferIndex;
    int32_t materialID;

    MeshPart(int32_t bufferIndex, int32_t materialID) : bufferIndex(bufferIndex), materialID(materialID) {}
};

struct MeshInstanceUBO
{
    glm::mat4 model = glm::mat4(1.0f);
    // Color ID for mouse picking
    int32_t meshColorID;
    int32_t textureID = 0;
    int32_t instanceColorID;
    int32_t useTexture = true;
    glm::vec3 albedo = glm::vec3(0.5f);
    float metallic = 0.0f;
    float roughness = 1.0f;
    float padding[3];

    MeshInstanceUBO(int32_t meshColorID, int32_t instanceColorID, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f)) : meshColorID(meshColorID), instanceColorID(instanceColorID)
    {
        model = glm::translate(model, pos);
        model = glm::scale(model, scale);
    }
};

struct PhysicsDebugUBO
{
    glm::mat4 model;
    glm::vec3 scale;
    int32_t havePhysicsInfo;

    PhysicsDebugUBO() : model(1.0f), scale(1.0f), havePhysicsInfo(false) {}
};

struct MeshInstance
{
    const uint64_t UUID;
    MeshInstanceUBO UBO;
    std::unique_ptr<PhysicsDebugDrawer> physicsDebugDrawer;
    std::unique_ptr<PhysicsInfo> physicsInfo;
    PhysicsDebugUBO physicsDebugUBO;
    std::unique_ptr<vkn::Buffer> physicsDebugUBOBuffer;

    MeshInstance(uint64_t UUID, MeshInstanceUBO&& UBO) : UBO(UBO), UUID(UUID)
    {
        vkn::BufferInfo bufferInfo = { sizeof(PhysicsDebugUBO), vk::WholeSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        physicsDebugUBOBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    }
    MeshInstance& operator=(const MeshInstance& other)
    {
        this->UBO = other.UBO;
        return *this;
    }
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
    std::weak_ptr<void> ptr;

    explicit Resource(std::string& path)
    {
        this->filePath = path;
        this->fileName = path.substr(path.find_last_of("/\\") + 1, path.rfind('.') - path.find_last_of("/\\") - 1);
    }
};

#endif