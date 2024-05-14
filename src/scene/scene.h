#ifndef SCENE_H
#define SCENE_H

#include "../common.h"
#include "scene.h"
#include "camera.h"
#include "mesh/meshModel.h"
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
#include "../pipeline/lineRender.h"
#include "../vulkan/image.h"
#include "../../imgui/imgui_impl_vulkan.h"
#include "physics.h"

class Scene
{
    friend class Engine;
    friend class UI;
    friend class Viewport;
    friend class SceneSerializer;

    vk::CommandPool m_commandPool;
    std::array<vk::CommandBuffer, MAX_FRAME> m_commandBuffers;

    std::unique_ptr<vkn::Buffer> m_meshInstanceDataBuffer;
    std::unique_ptr<vkn::Buffer> m_dirLightDataBuffer;
    std::unique_ptr<vkn::Buffer> m_pointLightDataBuffer;

    ShadowMap m_shadowMap;
    std::vector<std::unique_ptr<ShadowCubemap>> m_shadowCubemaps;

    std::vector<MeshModel> m_meshes;
    Mesh m_envCube;
    std::string m_hdriFilePath;
    std::unique_ptr<vkn::Image> m_envMap;
    std::unique_ptr<EnvCubemap> m_envCubemap;
    std::unique_ptr<EnvCubemap> m_irradianceCubemap;
    std::unique_ptr<PrefilteredCubemap> m_prefilteredCubemap;
    Mesh m_brdfLutSquare;
    vkn::Image m_brdfLut;
    float m_iblExposure;

    std::vector<PointLightUBO> m_pointLights;
    DirLightUBO m_dirLightUBO;
    float m_dirLightNearPlane = 1.0f;
    float m_dirLightFarPlane = 45.0f;
    float m_dirLightDistance = 40.0f;
    float m_dirLightSize = 10.0f;
    glm::mat4 m_dirLightRot = glm::mat4(1.0f);
    glm::vec3 m_dirLightPos = glm::vec3(0.0f, m_dirLightDistance, 0.0f);

    std::vector<std::unique_ptr<vkn::Image>> m_albedoTextures;
    std::vector<std::unique_ptr<vkn::Image>> m_normalTextures;
    std::vector<std::unique_ptr<vkn::Image>> m_metallicTextures;
    std::vector<std::unique_ptr<vkn::Image>> m_roughnessTextures;
    std::vector<Resource> m_resources;

    Camera m_camera;
    std::unique_ptr<vkn::Buffer> m_shadowMapViewSpaceProjBuffer;
    glm::mat4 m_shadowMapViewProj;
    std::unique_ptr<vkn::Buffer> m_shadowCubemapProjBuffer;
    glm::mat4 m_shadowCubemapProj;

    int32_t m_selectedMeshID;
    int32_t m_selectedMeshInstanceID;
    int32_t m_selectedLightID;

    bool m_showLightIcon;
    bool m_meshDirtyFlag;
    bool m_lightDirtyFlag;
    bool m_shadowShadowCubemapDirtyFlag;
    bool m_resourceDirtyFlag;
    bool m_envCubemapDirtyFlag;

    std::string m_saveFilePath;

    Physics m_physics;
    bool m_isPlaying;

    void AddResource(std::string& filePath);
    void LoadMaterials(const std::string& modelPath, const std::vector<MaterialFilePath>& materials);
    void AddMeshInstance(uint32_t id, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
    void AddLight();
    void AddEnvironmentMap(const std::string& hdriFilePath);
    void DeleteMeshInstance();
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
    size_t GetLightCount() { return m_pointLights.size(); }
    const std::vector<MeshModel>& GetMeshes() { return m_meshes; }
    // TODO: safety
    MeshModel& GetSelectedMesh() { return m_meshes[m_selectedMeshID]; }
    MeshInstanceUBO& GetSelectedMeshInstanceUBO() { return m_meshes[m_selectedMeshID].meshInstanceUBOs_[m_selectedMeshInstanceID]; }
    MeshInstanceUBO& GetMeshInstanceUBO(int32_t meshID, int32_t instanceID) { return m_meshes[meshID].meshInstanceUBOs_[instanceID]; }
    void SelectByColorID(int32_t meshID, int32_t instanceID);
    void Play();
    void Stop();
    ~Scene();
};

#endif