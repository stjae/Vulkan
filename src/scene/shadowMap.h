#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "../vulkan/sync.h"
#include "../vulkan/image.h"
#include "../scene/mesh.h"
#include "../pipeline/shadowMap.h"

inline static uint32_t shadowMapSize = 2048;

class ShadowMap : public vkn::Image
{
    ShadowMapPushConstants m_pushConstants;
    vk::PipelineStageFlags m_waitStage = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    void CreateFramebuffer();

public:
    void CreateShadowMap(vk::CommandBuffer& commandBuffer);
    void DrawShadowMap(vk::CommandBuffer& commandBuffer, std::vector<Mesh>& meshes);
};

#endif