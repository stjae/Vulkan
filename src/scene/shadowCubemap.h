#ifndef SHADOWCUBEMAP_H
#define SHADOWCUBEMAP_H

#include "../common.h"
#include "mesh/meshModel.h"
#include "../vulkan/cubemap.h"
#include "../pipeline/meshRender.h"
#include "../pipeline/shadowCubemap.h"
#include "../vulkan/command.h"

inline static uint32_t shadowCubemapSize = 1024;

class ShadowCubemap : public vkn::Cubemap
{
    friend class Scene;

    ShadowCubemapPushConstants m_pushConstants;
    vkn::Image m_depthImage;
    vk::PipelineStageFlags m_waitStage = { vk::PipelineStageFlagBits::eTopOfPipe };

    inline static vk::CommandPool s_commandPool;
    inline static std::array<vk::CommandBuffer, MAX_FRAME> m_commandBuffers;

    void UpdateCubemapFace(uint32_t faceIndex, vk::CommandBuffer& commandBuffer, int lightIndex, std::vector<PointLightUBO>& lights, std::vector<MeshModel>& meshes);
    void CreateFramebuffer();
    void CreateDepthImage(vk::CommandBuffer& commandBuffer);

public:
    void CreateShadowMap(vk::CommandBuffer& commandBuffer);
    void DrawShadowMap(int lightIndex, std::vector<PointLightUBO>& lights, std::vector<MeshModel>& meshes);
};

#endif
