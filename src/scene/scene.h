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

enum RESOURCETYPE {
    MESH,
    TEXTURE,
};

struct Texture
{
    vkn::Image image;
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

    std::unique_ptr<vkn::Buffer> meshInstanceDataBuffer_;
    std::unique_ptr<vkn::Buffer> lightDataBuffer_;

    std::vector<ShadowMap> shadowMaps_;
    bool shadowMapDirtyFlag_;
    bool showLightIcon_;

    void CreateDummyTexture(Texture& texture);

    std::vector<Mesh> meshes_;
    bool meshDirtyFlag_;
    std::vector<LightData> lights_;
    bool lightDirtyFlag_;
    std::vector<Texture> textures_;
    std::vector<Texture> diffuseTextures_;
    std::vector<Texture> normalTextures_;
    std::vector<Resource> resources_;
    Camera camera_;
    std::unique_ptr<vkn::Buffer> shadowMapCameraBuffer_;
    CameraData shadowMapCameraData_;
    int32_t selectedMeshID_;
    int32_t selectedMeshInstanceID_;
    int32_t selectedLightID_;

    void AddResource(std::string& filePath);
    void LoadMaterials(const std::string& modelPath, const std::vector<Material>& materials);
    void AddMeshInstance(uint32_t id, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
    void CreateTexture(const std::string& filePath, Texture& texture, vk::Format format = vk::Format::eR8G8B8A8Srgb);
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
    int GetLightCount() { return lights_.size(); }
    void SelectByColorID(Viewport& viewport);
    ~Scene();
};

#endif