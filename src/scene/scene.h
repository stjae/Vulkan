#ifndef SCENE_H
#define SCENE_H

#include "../common.h"
#include "scene.h"
#include "camera.h"
#include "mesh/meshModel.h"
#include "light.h"
#include "shadowCubemap.h"
#include "envCubemap.h"
#include "prefilteredCubemap.h"
#include "../vulkan/swapchain.h"
#include "../vulkan/command.h"
#include "../pipeline/envCubemap.h"
#include "../pipeline/irradianceCubemap.h"
#include "../pipeline/prefilteredCubemap.h"
#include "../pipeline/brdfLut.h"
#include "../pipeline/skyboxRender.h"
#include "../vulkan/image.h"
#include "../../imgui/imgui_impl_vulkan.h"

enum RESOURCETYPE {
    MESH,
    // TEXTURE,
};

struct Resource
{
    std::string fileName;
    std::string fileFormat;
    std::string filePath;
    RESOURCETYPE resourceType;
    void* resource;

    explicit Resource(std::string& path);
};

class Scene
{
    friend class UI;
    friend class Viewport;

    vk::CommandPool commandPool_;
    vk::CommandBuffer commandBuffer_;

    std::unique_ptr<vkn::Buffer> meshInstanceDataBuffer_;
    std::unique_ptr<vkn::Buffer> lightDataBuffer_;

    std::vector<ShadowCubemap> shadowMaps_;

    std::vector<MeshModel> meshes_;
    Mesh envCube_;
    std::unique_ptr<vkn::Image> envMap_;
    std::unique_ptr<EnvCubemap> envCubemap_;
    std::unique_ptr<EnvCubemap> irradianceCubemap_;
    std::unique_ptr<PrefilteredCubemap> prefilteredCubemap_;
    Mesh brdfLutSquare_;
    vkn::Image brdfLut_;

    std::vector<LightData> lights_;
    std::vector<vkn::Image> albedoTextures_;
    std::vector<vkn::Image> normalTextures_;
    std::vector<vkn::Image> metallicTextures_;
    std::vector<vkn::Image> roughnessTextures_;
    std::vector<Resource> resources_;

    Camera camera_;
    std::unique_ptr<vkn::Buffer> shadowMapCameraBuffer_;
    CameraData shadowMapCameraData_;

    int32_t selectedMeshID_;
    int32_t selectedMeshInstanceID_;
    int32_t selectedLightID_;

    bool shadowMapDirtyFlag_;
    bool showLightIcon_;
    bool lightDirtyFlag_;
    bool meshDirtyFlag_;

    void AddResource(std::string& filePath);
    void LoadMaterials(const std::string& modelPath, const std::vector<Material>& materials);
    void AddMeshInstance(uint32_t id, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
    void AddLight();
    void AddEnvironmentMap();
    void DeleteMesh();
    void DeleteLight();
    void HandleLightDuplication();
    void HandleMeshDuplication();
    void UpdateCamera();
    void UpdateLight();
    void UpdateMesh();
    void UpdateShadowMap();
    void UpdateDescriptorSet();

public:
    Scene();
    void Update();
    size_t GetInstanceCount();
    size_t GetLightCount() { return lights_.size(); }
    const std::vector<MeshModel>& GetMeshes() { return meshes_; }
    MeshInstance& GetSelectedMeshInstance() { return meshes_[selectedMeshID_].meshInstances_[selectedMeshInstanceID_]; }
    MeshInstance& GetMeshInstance(int32_t meshID, int32_t instanceID) { return meshes_[meshID].meshInstances_[instanceID]; }
    void SelectByColorID(int32_t meshID, int32_t instanceID);
    ~Scene();
};

#endif