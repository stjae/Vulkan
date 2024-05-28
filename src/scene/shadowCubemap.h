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

    vk::CommandPool m_commandPool;
    std::array<vk::CommandBuffer, MAX_FRAME> m_commandBuffers;

    void UpdateCubemapFace(uint32_t faceIndex, int lightIndex, std::vector<PointLightUBO>& lights, std::vector<std::shared_ptr<Mesh>>& meshes);
    void CreateFramebuffer();
    void CreateDepthImage();

public:
    void CreateShadowMap();
    void DrawShadowMap(int lightIndex, std::vector<PointLightUBO>& lights, std::vector<std::shared_ptr<Mesh>>& meshes);
    ~ShadowCubemap();
};

typedef struct PointLightUBO_
{
    glm::mat4 model;
    glm::vec3 color;
    float padding;

    PointLightUBO_() : model(glm::mat4(1.0f)), color(1.0f) {}
} PointLightUBO_;

#endif
