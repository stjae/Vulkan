#ifndef SCENE_H
#define SCENE_H

#include "../common.h"
#include "scene.h"
#include "camera.h"
#include "mesh.h"
#include "light.h"
#include "shadowMap.h"
#include "../engine/viewport.h"
#include "../vulkan/swapchain.h"
#include "../vulkan/command.h"
#include "../vulkan/pipeline.h"
#include "../../imgui/imgui_impl_vulkan.h"

struct Texture
{
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
    friend class UI;

    vk::CommandPool commandPool_;
    vk::CommandBuffer commandBuffer_;

    std::unique_ptr<Buffer> meshInstanceDataBuffer_;
    std::unique_ptr<Buffer> lightDataBuffer_;

    std::vector<ShadowMap> shadowMaps_;
    bool shadowMapDirtyFlag_;
    bool showLightIcon_;

    void CreateDummyTexture();

    std::vector<Mesh> meshes_;
    bool meshDirtyFlag_;
    std::vector<LightData> lights_;
    bool lightDirtyFlag_;
    std::vector<std::shared_ptr<Texture>> textures_;
    std::vector<Resource> resources_;
    Camera camera_;
    std::unique_ptr<Buffer> shadowMapCameraBuffer_;
    CameraData shadowMapCameraData_;
    int32_t selectedMeshID_;
    int32_t selectedMeshInstanceID_;
    int32_t selectedLightID_;

    void AddResource(std::string& filePath);
    void AddMeshInstance(uint32_t id, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
    void AddTexture(const std::string& filePath);
    void AddLight();
    void DeleteMesh();
    void DeleteLight();

public:
    Scene();
    void Update();
    size_t GetInstanceCount();
    const std::vector<Mesh>& GetMeshes() { return meshes_; }
    MeshInstance& GetSelectedMeshInstance() { return meshes_[selectedMeshID_].meshInstances_[selectedMeshInstanceID_]; }
    MeshInstance& GetMeshInstance(int32_t meshID, int32_t instanceID) { return meshes_[meshID].meshInstances_[instanceID]; }
    void SelectByColorID(Viewport& viewport);
    ~Scene();
};

#endif