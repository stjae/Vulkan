#ifndef SHADOWCUBEMAP_H
#define SHADOWCUBEMAP_H

#include "../common.h"
#include "mesh.h"
#include "../vulkan/cubemap.h"
#include "../pipeline/meshRender.h"
#include "../pipeline/shadowCubemap.h"
#include "../vulkan/command.h"

inline static uint32_t shadowCubemapSize = 1024;
typedef struct PointLightUBO_ PointLightUBO;

class ShadowCubemap : public vkn::Cubemap
{
    friend class Scene;

    ShadowCubemapPushConstants m_pushConstants;
    vkn::Image m_depthImage;
    vk::PipelineStageFlags m_waitStage = { vk::PipelineStageFlagBits::eTopOfPipe };

    inline static vk::CommandPool s_commandPool;
    inline static std::array<vk::CommandBuffer, MAX_FRAME> m_commandBuffers;

    void UpdateCubemapFace(uint32_t faceIndex, vk::CommandBuffer& commandBuffer, int lightIndex, std::vector<PointLightUBO>& lights, std::vector<std::shared_ptr<Mesh>>& meshes);
    void CreateFramebuffer();
    void CreateDepthImage(vk::CommandBuffer& commandBuffer);

public:
    void CreateShadowMap(vk::CommandBuffer& commandBuffer);
    void DrawShadowMap(int lightIndex, std::vector<PointLightUBO>& lights, std::vector<std::shared_ptr<Mesh>>& meshes);
};

typedef struct PointLightUBO_
{
    glm::mat4 model;
    glm::vec3 pos;
    float padding0;
    glm::vec3 color;
    float padding1;

    PointLightUBO_() : model(glm::mat4(1.0f)), pos(glm::vec3(0.0f)), color(1.0f), padding0(0.0f), padding1(0.0f) {}
    explicit PointLightUBO_(glm::vec3&& pos) : model(glm::mat4(1.0f)), pos(pos), color(1.0f), padding0(0.0f), padding1(0.0f) {}
} PointLightUBO_;

#endif
