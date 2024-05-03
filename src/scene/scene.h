#ifndef SCENE_H
#define SCENE_H

#include "../common.h"
#include "scene.h"
#include "camera.h"
#include "mesh/meshModel.h"
#include "dataType.h"
#include "shadowMap.h"
#include "shadowCubemap.h"
#include "envCubemap.h"
#include "prefilteredCubemap.h"
#include "../vulkan/swapchain.h"
#include "../vulkan/command.h"
#include "../pipeline/shadowMap.h"
#include "../pipeline/envCubemap.h"
#include "../pipeline/irradianceCubemap.h"
#include "../pipeline/prefilteredCubemap.h"
#include "../pipeline/brdfLut.h"
#include "../pipeline/skyboxRender.h"
#include "../vulkan/image.h"
#include "../../imgui/imgui_impl_vulkan.h"
#include "physics.h"

class Scene
{
    friend class UI;
    friend class Viewport;
    friend class SceneSerializer;

    vk::CommandPool commandPool_;
    vk::CommandBuffer commandBuffer_;

    std::unique_ptr<vkn::Buffer> meshInstanceDataBuffer_;
    std::unique_ptr<vkn::Buffer> dirLightDataBuffer_;
    std::unique_ptr<vkn::Buffer> pointLightDataBuffer_;

    ShadowMap shadowMap_;
    std::vector<ShadowCubemap> shadowCubemaps_;

    std::vector<MeshModel> meshes_;
    Mesh envCube_;
    std::string hdriFilePath_;
    std::unique_ptr<vkn::Image> envMap_;
    std::unique_ptr<EnvCubemap> envCubemap_;
    std::unique_ptr<EnvCubemap> irradianceCubemap_;
    std::unique_ptr<PrefilteredCubemap> prefilteredCubemap_;
    Mesh brdfLutSquare_;
    vkn::Image brdfLut_;
    float iblExposure_;

    std::vector<PointLightUBO> pointLights_;
    DirLightUBO dirLightUBO_;
    float dirLightNearPlane_ = 1.0f;
    float dirLightFarPlane_ = 45.0f;
    float dirLightDistance_ = 40.0f;
    float dirLightSize_ = 10.0f;
    glm::mat4 dirLightRot_ = glm::mat4(1.0f);
    glm::vec3 dirLightPos_ = glm::vec3(0.0f, dirLightDistance_, 0.0f);

    std::vector<vkn::Image> albedoTextures_;
    std::vector<vkn::Image> normalTextures_;
    std::vector<vkn::Image> metallicTextures_;
    std::vector<vkn::Image> roughnessTextures_;
    std::vector<Resource> resources_;

    Camera camera_;
    std::unique_ptr<vkn::Buffer> shadowMapViewSpaceProjBuffer_;
    glm::mat4 shadowMapViewProj_;
    std::unique_ptr<vkn::Buffer> shadowCubemapProjBuffer_;
    glm::mat4 shadowCubemapProj_;

    int32_t selectedMeshID_;
    int32_t selectedMeshInstanceID_;
    int32_t selectedLightID_;

    bool showLightIcon_;
    bool meshDirtyFlag_;
    bool lightDirtyFlag_;
    bool shadowShadowCubemapDirtyFlag_;
    bool resourceDirtyFlag_;
    bool envCubemapDirtyFlag_;

    std::string saveFilePath_;

    Physics physics_;

    void AddResource(std::string& filePath);
    void LoadMaterials(const std::string& modelPath, const std::vector<MaterialFilePath>& materials);
    void AddMeshInstance(uint32_t id, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
    void AddLight();
    void AddEnvironmentMap(const std::string& hdriFilePath);
    void DeleteMesh();
    void DeletePointLight();
    void HandlePointLightDuplication();
    void HandleMeshDuplication();
    void UpdateCamera();
    void UpdatePointLight();
    void UpdateMesh();
    void UpdateShadowMap();
    void UpdateShadowCubemaps();
    void UpdateDescriptorSet();
    void UpdateUniformDescriptors();
    void UpdateTextureDescriptors();
    void UpdateEnvCubemapDescriptors();
    void InitScene();
    void InitHdri();

public:
    Scene();
    void Update();
    size_t GetInstanceCount();
    size_t GetLightCount() { return pointLights_.size(); }
    const std::vector<MeshModel>& GetMeshes() { return meshes_; }
    MeshInstanceUBO& GetSelectedMeshInstance() { return meshes_[selectedMeshID_].meshInstances_[selectedMeshInstanceID_]; }
    MeshInstanceUBO& GetMeshInstance(int32_t meshID, int32_t instanceID) { return meshes_[meshID].meshInstances_[instanceID]; }
    void SelectByColorID(int32_t meshID, int32_t instanceID);
    ~Scene();
};

#endif