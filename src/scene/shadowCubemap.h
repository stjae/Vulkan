// Shadow cubemap creation and update
// シャドウキューブマップの生成と更新

#ifndef SHADOWCUBEMAP_H
#define SHADOWCUBEMAP_H

#include "../common.h"
#include "mesh.h"
#include "../vulkan/cubemap.h"
#include "../pipeline/meshRenderPipeline.h"
#include "../pipeline/shadowCubemapPipeline.h"
#include "../vulkan/command.h"
#include "light.h"

inline static uint32_t shadowCubemapSize = 1024;

class ShadowCubemap : public vkn::Cubemap
{
    ShadowCubemapPushConstants m_pushConstants;
    vkn::Image m_depthImage;
    vk::PipelineStageFlags m_waitStage = { vk::PipelineStageFlagBits::eTopOfPipe };

    inline static float s_zNear = 0.001f;
    inline static float s_zFar = 128.0f;
    inline static std::unique_ptr<vkn::Buffer> s_shadowCubemapProjBuffer;
    inline static glm::mat4 s_shadowCubemapProj;

    void UpdateCubemapFace(uint32_t faceIndex, int lightIndex, PointLight& light, std::vector<std::unique_ptr<Mesh>>& meshes, const vk::CommandBuffer& commandBuffer);
    void CreateFramebuffer();
    void CreateDepthImage(const vk::CommandBuffer& commandBuffer);

public:
    void CreateShadowMap(const vk::CommandBuffer& commandBuffer);
    void DrawShadowMap(int lightIndex, PointLight& light, std::vector<std::unique_ptr<Mesh>>& meshes, const vk::CommandBuffer& commandBuffer);
    static void CreateProjBuffer();
    static void DestroyBuffer();
};

#endif
