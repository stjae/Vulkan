// Creation and update of prefiltered cubemap
// プリフィルタされたキューブマップの生成と更新

#ifndef PREFILTEREDCUBEMAP_H
#define PREFILTEREDCUBEMAP_H

#include "../vulkan/pipeline.h"
#include "../scene/envCubemap.h"
#include "mesh.h"

class PrefilteredCubemap : public EnvCubemap
{
    PrefilteredCubemapPushConstants m_pushConstants;

    void DrawPrefilteredCubemap(uint32_t mipSize, float roughness, const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, const vk::CommandBuffer& commandBuffer);
    void DrawPrefilteredCubemapFace(float roughness, uint32_t faceIndex, const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, const vk::CommandBuffer& commandBuffer);

public:
    void DrawMipmap(const vk::CommandBuffer& commandBuffer, const Mesh& envCube, const vkn::Pipeline& cubemapPipeline);
};

#endif
