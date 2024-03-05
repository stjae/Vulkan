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

    int width, height;
    size_t size;
    int32_t index;

    ~Texture() { ImGui_ImplVulkan_RemoveTexture(descriptorSet); }
};

struct Resource
{
    std::string fileName;
    std::string fileFormat;
    std::string filePath;
    RESOURCETYPE resourceType;
    void* resource;

    Resource(std::string& path);
};

class Scene
{
    friend class Engine;
    friend class UI;
    friend class Viewport;

    vk::CommandPool commandPool_;
    vk::CommandBuffer commandBuffer_;

    std::unique_ptr<Buffer> meshInstanceDataBuffer_;
    std::unique_ptr<Buffer> lightDataBuffer_;

    void CreateDummyTexture();

public:
    std::vector<Mesh> meshes;
    std::vector<LightData> lights;
    std::vector<std::shared_ptr<Texture>> textures;
    std::vector<Resource> resources;
    Camera camera;
    int32_t selectedMeshID;
    int32_t selectedMeshInstanceID;
    int32_t selectedLightID;

    Scene();
    void AddResource(std::string& filePath);
    void AddMesh(uint32_t id);
    void AddTexture(const std::string& filePath);
    void DeleteMesh();
    void Update();
    size_t GetInstanceCount();
    ~Scene();
};

#endif