#ifndef CASCADED_SHADOW_MAP_H
#define CASCADED_SHADOW_MAP_H

#include "../vulkan/sync.h"
#include "../vulkan/image.h"
#include "../scene/mesh.h"
#include "../pipeline/shadowMap.h"
#include "shadowMap.h"
#include "light.h"

struct CascadeUBO
{
    float depth[SHADOW_MAP_CASCADE_COUNT];
    glm::mat4 viewProj[SHADOW_MAP_CASCADE_COUNT];
    glm::vec3 lightDir;
    int debug;
    glm::vec3 color{ 1.0f };
    float intensity = 1.0f;
};

class Cascade : public vkn::Image
{
    friend class CascadedShadowMap;

    ShadowMapPushConstants m_pushConstants = {};
    float m_depth;
    glm::mat4 m_viewProj = {};

public:
    void Create(int index, const vkn::Image& depthImage);
    void Draw(int index, vkn::Image& depthImage, std::vector<std::shared_ptr<Mesh>>& meshes, const vk::CommandBuffer& commandBuffer);
};

class CascadedShadowMap
{
    friend class UI;

    std::array<Cascade, SHADOW_MAP_CASCADE_COUNT> m_cascades;
    vkn::Image m_depthImage;
    std::unique_ptr<vkn::Buffer> m_UBOBuffer;
    std::unique_ptr<vkn::Buffer> m_UBOStagingBuffer;
    CascadeUBO m_UBO;
    vk::PipelineStageFlags m_waitStage = { vk::PipelineStageFlagBits::eVertexShader };

public:
    CascadedShadowMap();
    void Create();
    void UpdateCascades(Camera* camera, const DirLight& dirLight);
    void UpdateUBO(const DirLight& dirLight, const vk::CommandBuffer& commandBuffer);
    void Draw(std::vector<std::shared_ptr<Mesh>>& meshes, const vk::CommandBuffer& commandBuffer);
};

#endif
