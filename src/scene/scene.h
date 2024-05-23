#ifndef SCENE_H
#define SCENE_H

#include "../common.h"
#include "camera.h"
#include "mesh.h"
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

typedef struct DirLightUBO_ DirLightUBO;
typedef struct DirLightUBO_
{
    glm::vec3 dir;
    float intensity = 1.0f;
    glm::vec3 color = glm::vec3(1.0f);
} DirLightUBO_;
typedef struct Resource_ Resource;

class Scene
{
    friend class UI;
    friend class Viewport;
    friend class SceneSerializer;

    std::unique_ptr<vkn::Buffer> m_cameraStagingBuffer;

    vk::CommandPool m_commandPool;
    std::array<vk::CommandBuffer, MAX_FRAME> m_commandBuffers;
    std::array<vk::CommandBuffer, MAX_FRAME> m_cameraCommandBuffers;
    std::array<vk::CommandPool, 4> m_imageLoadCommandPools;
    std::array<vk::CommandBuffer, 4> m_imageLoadCommandBuffers;

    std::unique_ptr<vkn::Buffer> m_meshInstanceDataBuffer;
    std::unique_ptr<vkn::Buffer> m_dirLightDataBuffer;
    std::unique_ptr<vkn::Buffer> m_pointLightDataBuffer;

    ShadowMap m_shadowMap;
    std::vector<std::unique_ptr<ShadowCubemap>> m_shadowCubemaps;

    std::vector<std::shared_ptr<Mesh>> m_meshes;
    std::vector<Mesh> m_meshCopies;
    Mesh m_envCube;
    std::string m_hdriFilePath;
    std::unique_ptr<vkn::Image> m_envMap;
    std::unique_ptr<EnvCubemap> m_envCubemap;
    std::unique_ptr<EnvCubemap> m_irradianceCubemap;
    std::unique_ptr<PrefilteredCubemap> m_prefilteredCubemap;
    Mesh m_brdfLutSquare;
    vkn::Image m_brdfLut;
    float m_iblExposure = 1.0f;

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

    int32_t m_selectedMeshID = -1;
    int32_t m_selectedMeshInstanceID = -1;
    int32_t m_selectedLightID = -1;

    bool m_showLightIcon = true;
    bool m_meshDirtyFlag = true;
    bool m_lightDirtyFlag = true;
    bool m_shadowShadowCubemapDirtyFlag = true;
    bool m_resourceDirtyFlag = true;
    bool m_envCubemapDirtyFlag = true;

    std::string m_sceneFolderPath;
    std::string m_sceneFilePath;

    Physics m_physics;
    bool m_isPlaying = false;
    bool m_isStartUp = true;

    void AddResource(std::string& filePath);
    void LoadMaterials(const std::string& modelPath, const std::vector<MaterialFilePath>& materials);
    void AddMeshInstance(Mesh& mesh, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
    void AddMeshInstance(Mesh& mesh, uint64_t UUID);
    void AddPhysics(Mesh& mesh, MeshInstance& meshInstance, PhysicsInfo& physicsInfo);
    void DeletePhysics(MeshInstance& meshInstance);
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
    void UpdatePhysicsDebug();
    size_t GetInstanceCount();
    size_t GetLightCount() { return m_pointLights.size(); }
    const std::vector<std::shared_ptr<Mesh>>& GetMeshes() { return m_meshes; }
    // TODO: std::optional
    Mesh& GetSelectedMesh() { return *m_meshes[m_selectedMeshID]; }
    MeshInstance& GetSelectedMeshInstance() const { return *m_meshes[m_selectedMeshID]->m_meshInstances[m_selectedMeshInstanceID]; }
    MeshInstanceUBO& GetSelectedMeshInstanceUBO() { return m_meshes[m_selectedMeshID]->m_meshInstances[m_selectedMeshInstanceID]->UBO; }
    MeshInstanceUBO& GetMeshInstanceUBO(int32_t meshID, int32_t instanceID) { return m_meshes[meshID]->m_meshInstances[instanceID]->UBO; }
    void SelectByColorID(int32_t meshID, int32_t instanceID);
    void UnselectAll();
    bool IsPlaying() { return m_isPlaying; }

    void Play();
    void Stop();
    ~Scene();
    void CopyMeshInstances();
    void RevertMeshInstances();
};

typedef struct Resource_
{
    std::string filePath;
    std::string fileName;
    std::weak_ptr<void> ptr;

    explicit Resource_(std::string& path)
    {
        this->filePath = path;
        this->fileName = path.substr(path.find_last_of("/\\") + 1, path.rfind('.') - path.find_last_of("/\\") - 1);
    }
} Resource_;

#endif