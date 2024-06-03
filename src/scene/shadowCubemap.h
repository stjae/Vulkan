#ifndef SHADOWCUBEMAP_H
#define SHADOWCUBEMAP_H

#include "../common.h"
#include "mesh.h"
#include "../vulkan/cubemap.h"
#include "../pipeline/meshRender.h"
#include "../pipeline/shadowCubemap.h"
#include "../vulkan/command.h"
#include "light.h"

inline static uint32_t shadowCubemapSize = 1024;

class ShadowCubemap : public vkn::Cubemap
{
    ShadowCubemapPushConstants m_pushConstants;
    vkn::Image m_depthImage;
    vk::PipelineStageFlags m_waitStage = { vk::PipelineStageFlagBits::eTopOfPipe };

    vk::CommandPool m_commandPool;
    std::array<vk::CommandBuffer, MAX_FRAME> m_commandBuffers;
    inline static float s_zNear = 0.001f;
    inline static float s_zFar = 1024.0f;
    inline static std::unique_ptr<vkn::Buffer> s_shadowCubemapProjBuffer;
    inline static glm::mat4 s_shadowCubemapProj;

    void UpdateCubemapFace(uint32_t faceIndex, int lightIndex, PointLight& lights, std::vector<std::shared_ptr<Mesh>>& meshes);
    void CreateFramebuffer();
    void CreateDepthImage();

public:
    void CreateShadowMap();
    void DrawShadowMap(int lightIndex, PointLight& light, std::vector<std::shared_ptr<Mesh>>& meshes);
    static void CreateProjBuffer();
    static void DestroyBuffer();
    ~ShadowCubemap();
};

#endif
