#ifndef SCENE_H
#define SCENE_H

#include "../common.h"
#include "scene.h"
#include "camera.h"
#include "mesh.h"
#include "light.h"
#include "../vulkan/swapchain.h"
#include "../vulkan/command.h"
#include "../../imgui/imgui_impl_vulkan.h"

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
    // pointer to resource
    std::shared_ptr<void> resource;

    Resource(std::string& path)
    {
        this->filePath = path;
        this->fileName = path.substr(path.rfind('/') + 1, path.rfind('.') - path.rfind('/') - 1);
        this->fileFormat = path.substr(path.rfind('.') + 1, path.size());
    }
};

class Scene
{
    friend class Engine;
    friend class UI;
    friend class Viewport;

    vk::CommandPool commandPool_;
    vk::CommandBuffer commandBuffer_;

    std::unique_ptr<Buffer> meshInstanceDataBuffer_;
    std::vector<MeshInstanceData> meshInstanceData_;

    void CreateDummyTexture();
    void PrepareMesh(Mesh& mesh);

public:
    std::vector<Mesh> meshes;
    std::vector<std::shared_ptr<Texture>> textures;
    std::vector<Resource> resources;
    Camera camera;
    Light light;
    int32_t selectedMeshIndex = -1;

    Scene();
    void AddMesh(TypeEnum::Mesh type);
    void AddMesh(TypeEnum::Mesh type, const std::string& filePath);
    void AddTexture(const std::string& filePath);
    const char* GetMeshName(size_t index) const { return meshes[index].name_.c_str(); }
    void DeleteMesh(size_t index);
    void Update();
    ~Scene();
};

#endif