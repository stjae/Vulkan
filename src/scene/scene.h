#ifndef SCENE_H
#define SCENE_H

#include "../common.h"
#include "scene.h"
#include "camera.h"
#include "mesh.h"
#include "light.h"
#include "../API/swapchain.h"
#include "../API/command.h"

struct MeshUniformData
{
    glm::mat4 modelMat;
    int32_t meshID;
    int32_t textureID;
    bool useTexture;
};

struct Texture
{
    std::unique_ptr<Buffer> stagingBuffer;
    std::unique_ptr<Image> image;
    vk::DescriptorSet descriptorSet; // thumbnail for resource window

    int width{}, height{};
    size_t size{};
    int32_t index;

    ~Texture() { ImGui_ImplVulkan_RemoveTexture(descriptorSet); }
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
    friend class Engine;

    vk::CommandPool commandPool_;
    vk::CommandBuffer commandBuffer_;
    std::unique_ptr<Buffer> meshUniformBufferDynamic_;
    size_t meshUniformBufferDynamicOffset_;
    size_t meshUniformBufferDynamicSize_;

    void CreateMeshUniformBuffer();
    void RearrangeMeshUniformBuffer(size_t index) const;
    void CreateDummyTexture();
    void UpdateMeshUniformBuffer();
    void PrepareMesh(Mesh& mesh);

public:
    std::vector<Mesh> meshes;
    std::vector<std::shared_ptr<Texture>> textures;
    std::vector<Resource> resources;
    Camera camera;
    Light light;
    MeshUniformData* meshUniformData;
    int32_t meshSelected = -1;

    Scene();
    void AddMesh(TypeEnum::Mesh type);
    void AddMesh(TypeEnum::Mesh type, const std::string& filePath);
    void AddTexture(const std::string& filePath);
    size_t GetMeshUniformDynamicOffset() const { return meshUniformBufferDynamicOffset_; }
    const char* GetMeshName(size_t index) const { return meshes[index].name_.c_str(); }
    bool IsMeshSelected(size_t index) const { return meshes[index].isSelected_; }
    void DeleteMesh(size_t index);
    void Update();
    ~Scene();
};

#endif