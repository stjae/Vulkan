#ifndef SCENE_H
#define SCENE_H

#include "../common.h"
#include "camera.h"
#include "mesh.h"
#include "light.h"
#include "shadowMap.h"
#include "cascadedShadowMap.h"
#include "shadowCubemap.h"
#include "envCubemap.h"
#include "prefilteredCubemap.h"
#include "../vulkan/swapchain.h"
#include "../vulkan/command.h"
#include "../pipeline/colorID.h"
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
#include "../time.h"

typedef struct Resource_ Resource;

class Scene
{
    friend class Engine;
    friend class UI;
    friend class Viewport;
    friend class SceneSerializer;

    std::array<vk::CommandPool, 4> m_imageLoadCommandPools;
    std::array<vk::CommandBuffer, 4> m_imageLoadCommandBuffers;
    std::unique_ptr<vkn::Buffer> m_meshInstanceDataBuffer;

    // Shadow
    DirLight m_dirLight;
    CascadedShadowMap m_cascadedShadowMap;
    PointLight m_pointLight;
    std::vector<std::unique_ptr<ShadowCubemap>> m_shadowCubemaps;
    // Mesh
    std::vector<std::shared_ptr<Mesh>> m_meshes;
    // store meshes for scene resetting
    std::vector<Mesh> m_meshCopies;
    std::unordered_map<uint64_t, MeshInstance*> m_meshInstanceMap;

    // Environment Map
    Mesh m_envCube;
    std::string m_hdriFilePath;
    std::unique_ptr<vkn::Image> m_envMap;
    std::unique_ptr<vkn::Image> m_dummyEnvMap;
    std::unique_ptr<EnvCubemap> m_envCubemap;
    std::unique_ptr<EnvCubemap> m_irradianceCubemap;
    std::unique_ptr<PrefilteredCubemap> m_prefilteredCubemap;
    Mesh m_brdfLutSquare;
    vkn::Image m_brdfLut;
    float m_iblExposure = 1.0f;

    // Texture
    std::vector<std::unique_ptr<vkn::Image>> m_albedoTextures;
    std::vector<std::unique_ptr<vkn::Image>> m_normalTextures;
    std::vector<std::unique_ptr<vkn::Image>> m_metallicTextures;
    std::vector<std::unique_ptr<vkn::Image>> m_roughnessTextures;

    // Camera
    std::unique_ptr<MainCamera> m_mainCamera;
    MeshInstance* m_selectedCameraMeshInstance;
    // meshInstance UUID of selected camera
    uint64_t m_selectedCameraUUID = 0;

    std::vector<Resource> m_resources;
    int32_t m_selectedMeshID = -1;
    int32_t m_selectedMeshInstanceID = -1;
    int32_t m_selectedLightIndex = -1;
    std::string m_sceneFolderPath;
    std::string m_sceneFilePath;
    Physics m_physics;
    bool m_showLightIcon = true;
    bool m_isPlaying = false;
    bool m_isStartUp = true;

    void CreateCommandBuffers();
    void CreateMainCamera();
    void CreateShadowMap();
    void CreateEnvironmentMap(const vk::CommandBuffer& commandBuffer);
    void CreateDummyEnvMap(const vk::CommandBuffer& commandBuffer);
    void UpdateEnvCubemaps(const vk::CommandBuffer& commandBuffer);
    void AddResource(std::string& filePath, const vk::CommandBuffer& commandBuffer);
    void LoadMaterials(const std::string& modelPath, const std::vector<MaterialFilePath>& materials);
    void AddMeshInstance(Mesh& mesh, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
    void AddMeshInstance(Mesh& mesh, uint64_t UUID);
    void AddPhysics(Mesh& mesh, MeshInstance& meshInstance, PhysicsInfo& physicsInfo, const vk::CommandBuffer& commandBuffer);
    void DeletePhysics(MeshInstance& meshInstance);
    void AddPointLight(const vk::CommandBuffer& commandBuffer);
    void AddEnvironmentMap(const std::string& hdriFilePath, const vk::CommandBuffer& commandBuffer);
    void SelectDummyEnvMap(const vk::CommandBuffer& commandBuffer);
    void DeleteMeshInstance(Mesh& mesh, MeshInstance& instance);
    void DeletePointLight(const vk::CommandBuffer& commandBuffer);
    void HandlePointLightDuplication(const vk::CommandBuffer& commandBuffer);
    void HandleMeshDuplication(const vk::CommandBuffer& commandBuffer);
    void CopyMeshInstances();
    void RevertMeshInstances(const vk::CommandBuffer& commandBuffer);
    void UpdateMainCamera(const vk::CommandBuffer& commandBuffer);
    void UpdateCameraDescriptor(Camera* camera);
    void UpdatePointLight(const vk::CommandBuffer& commandBuffer);
    void UpdatePointLightBuffer();
    void UpdateMeshBuffer();
    void UpdateShadowMap(const vk::CommandBuffer& commandBuffer);
    void UpdateShadowCubemaps();
    void UpdateTextureDescriptors();
    void Clear(const vk::CommandBuffer& commandBuffer);
    void AddCamera(MeshInstance& instance);
    void SelectCamera(Camera* camera);
    void Play(const vk::CommandBuffer& commandBuffer);
    void Stop(const vk::CommandBuffer& commandBuffer);
    void Update(const vk::CommandBuffer& commandBuffer);
    void UpdatePhysicsDebug();

public:
    void Init(const vk::CommandBuffer& commandBuffer);
    size_t GetInstanceCount();
    const std::vector<std::shared_ptr<Mesh>>& GetMeshes() { return m_meshes; }
    Mesh& GetSelectedMesh() { return *m_meshes[m_selectedMeshID]; }
    MeshInstance& GetSelectedMeshInstance() const { return *m_meshes[m_selectedMeshID]->m_meshInstances[m_selectedMeshInstanceID]; }
    MeshInstance& GetMeshInstanceByID(uint64_t UUID) { return *m_meshInstanceMap[UUID]; }
    void SelectByColorID(int32_t meshID, int32_t instanceID);
    void UnselectAll();
    bool IsPlaying() { return m_isPlaying; }
    ~Scene();
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