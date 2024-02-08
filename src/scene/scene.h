#ifndef SCENE_H
#define SCENE_H

#include "../common.h"
#include "scene.h"
#include "camera.h"
#include "mesh.h"
#include "../API/swapchain.h"
#include "../API/command.h"
#include "../viewport.h"

struct MeshUniformData
{
    glm::mat4 modelMatrix;
    int32_t meshID;
    int32_t textureID;
};

struct Texture
{
    std::unique_ptr<Buffer> stagingBuffer;
    std::unique_ptr<Image> image;

    int width{}, height{};
    size_t size{};
    int32_t index;
};

struct Resource
{
    std::string fileName;
    std::string fileFormat;
    std::string filePath;
    TypeEnum::Resource resourceType;

    std::shared_ptr<void> resource;
};

class Scene
{
    vk::CommandPool commandPool_;
    vk::CommandBuffer commandBuffer_;
    std::unique_ptr<Buffer> modelUniformBufferDynamic_;
    size_t modelUniformBufferDynamicOffset_;
    size_t modelUniformBufferDynamicSize_;
    size_t meshCount_[3]{};

    void CreateUniformBuffer();
    void RearrangeUniformBuffer(size_t index) const;
    void CreateDummyTexture();
    void UpdateBuffer();
    void UpdateMesh();

public:
    std::vector<Mesh> meshes;
    std::vector<std::shared_ptr<Texture>> textures;
    std::vector<Resource> resources;
    Camera camera;
    MeshUniformData* meshUniformData;
    int32_t meshSelected = -1;

    Scene();
    void AddMesh(TypeEnum::Mesh type);
    void AddMesh(TypeEnum::Mesh type, const std::string& filePath);
    void AddTexture(const std::string& filePath);
    size_t GetMeshUniformDynamicOffset() { return modelUniformBufferDynamicOffset_; }
    const char* GetMeshName(size_t index) const { return meshes[index].name_.c_str(); }
    bool IsMeshSelected(size_t index) const { return meshes[index].isSelected_; }
    void PrepareMeshes();
    void DeleteMesh(size_t index);
    void Update(uint32_t frameIndex, const std::vector<ViewportFrame>& viewportFrames);
    void UpdateTextureID(size_t meshIndex, int32_t textureID);
    ~Scene();
};

#endif