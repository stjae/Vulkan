#ifndef SCENE_H
#define SCENE_H

#include "grid.h"
#include "camera.h"
#include "mesh.h"
#include "light.h"
#include "shadowMap.h"
#include "cascadedShadowMap.h"
#include "shadowCubemap.h"
#include "envCubemap.h"
#include "prefilteredCubemap.h"
#include "../vulkan/command.h"
#include "../vulkan/image.h"
#include "physics.h"

struct Resource;

class Scene
{
    friend class Engine;
    friend class UI;
    friend class Viewport;
    friend class Registry;
    friend class SceneSerializer;

    std::array<vk::CommandPool, 4> m_imageLoadCommandPools;
    std::array<vk::CommandBuffer, 4> m_imageLoadCommandBuffers;
    std::unique_ptr<vkn::Buffer> m_meshInstanceDataBuffer;

    Grid m_grid;
    int m_gridWidth = 10;
    bool m_showGrid = true;

    // Shadow
    DirLight m_dirLight;
    CascadedShadowMap m_cascadedShadowMap;
    PointLight m_pointLight;
    std::vector<std::unique_ptr<ShadowCubemap>> m_shadowCubemaps;
    // Mesh
    std::vector<std::unique_ptr<Mesh>> m_meshes;
    std::vector<Mesh> m_meshCopies;                              // store meshes for scene resetting
    std::unordered_map<uint64_t, MeshInstance*> m_meshInstances; // to search by uuid

    // Environment Map
    Mesh m_cube;
    std::string m_hdriFilePath;
    std::unique_ptr<vkn::Image> m_envMap;
    std::unique_ptr<vkn::Image> m_dummyEnvMap;
    std::unique_ptr<EnvCubemap> m_envCubemap;
    std::unique_ptr<EnvCubemap> m_irradianceCubemap;
    std::unique_ptr<PrefilteredCubemap> m_prefilteredCubemap;
    Mesh m_square;
    vkn::Image m_brdfLut;
    float m_iblExposure = 1.0f;

    // Texture
    std::vector<std::unique_ptr<vkn::Image>> m_albedoTextures;
    std::vector<std::unique_ptr<vkn::Image>> m_normalTextures;
    std::vector<std::unique_ptr<vkn::Image>> m_metallicTextures;
    std::vector<std::unique_ptr<vkn::Image>> m_roughnessTextures;

    // Camera
    MainCamera m_mainCamera;
    std::unordered_map<uint64_t, SubCamera*> m_cameras;
    Camera* m_viewportCamera = nullptr;
    Camera* m_playCamera = nullptr;

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

    void CreateImageLoadCommandBuffers();
    void CreateGrid(const vk::CommandBuffer& commandBuffer);
    void CreateMainCamera();
    void CreateShadowMap();
    void CreateEnvironmentMap(const vk::CommandBuffer& commandBuffer);
    void CreateDummyEnvMap(const vk::CommandBuffer& commandBuffer);
    void UpdateEnvCubemaps(const vk::CommandBuffer& commandBuffer);
    void AddResource(const vk::CommandBuffer& commandBuffer, std::string& filePath);
    void LoadMaterials(const std::string& modelPath, const std::string& modelName, const std::vector<MaterialFilePath>& materials);
    void AddMeshInstance(Mesh& mesh, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
    void AddMeshInstance(Mesh& mesh, uint64_t UUID);
    void AddPhysics(const vk::CommandBuffer& commandBuffer, Mesh& mesh, MeshInstance& meshInstance, PhysicsInfo& physicsInfo);
    void DeletePhysics(MeshInstance& meshInstance);
    void AddPointLight(const vk::CommandBuffer& commandBuffer);
    void AddEnvironmentMap(const vk::CommandBuffer& commandBuffer, const std::string& hdriFilePath);
    void SelectDummyEnvMap(const vk::CommandBuffer& commandBuffer);
    void DeleteMeshInstance(Mesh& mesh, MeshInstance& instance);
    void DeletePointLight();
    void DuplicatePointLight(const vk::CommandBuffer& commandBuffer, int lightIndex);
    void DuplicateMeshInstance(const vk::CommandBuffer& commandBuffer, int32_t meshID, int32_t meshInstanceID, glm::vec3 offset = { 0.0f, 0.0f, 0.0f });
    void CopyMeshInstances();
    void RevertMeshInstances(const vk::CommandBuffer& commandBuffer);
    void UpdateViewportCamera(const vk::CommandBuffer& commandBuffer);
    void UpdatePlayCamera(const vk::CommandBuffer& commandBuffer);
    void UpdateCameraDescriptor(Camera* camera);
    void UpdatePointLight(const vk::CommandBuffer& commandBuffer);
    void UpdatePointLightBuffer();
    void UpdateMeshBuffer();
    void UpdateShadowMap(const vk::CommandBuffer& commandBuffer);
    void UpdateShadowCubemaps();
    void UpdateTextureDescriptors();
    void Clear(const vk::CommandBuffer& commandBuffer);
    void AddCamera(MeshInstance& instance);
    void Play();
    void Stop(const vk::CommandBuffer& commandBuffer);
    void Update(const vk::CommandBuffer& commandBuffer);
    void UpdatePhysicsDebug();
    void DeleteMesh(int index);

public:
    void Init(const vk::CommandBuffer& commandBuffer);
    size_t GetInstanceCount();
    const std::vector<std::unique_ptr<Mesh>>& GetMeshes() { return m_meshes; }
    Mesh& GetSelectedMesh() { return *m_meshes[m_selectedMeshID]; }
    MeshInstance& GetSelectedMeshInstance() const { return *m_meshes[m_selectedMeshID]->m_meshInstances[m_selectedMeshInstanceID]; }
    MeshInstance& GetMeshInstanceByID(uint64_t UUID) { return *m_meshInstances[UUID]; }
    void SelectByColorID(int32_t meshID, int32_t instanceID);
    void UnselectAll();
    bool IsPlaying() const { return m_isPlaying; }
    ~Scene();
    void DeleteCamera(const uint64_t ID);
    void DeleteMatrials(int index);
};

struct Resource
{
    std::string filePath;
    std::string fileName;
    void* ptr;
    Resource(std::string& path, std::string& name) : filePath(path), fileName(name) {}
};

#endif