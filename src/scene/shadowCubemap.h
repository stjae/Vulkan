#ifndef SHADOWCUBEMAP_H
#define SHADOWCUBEMAP_H

#include "../common.h"
#include "light.h"
#include "mesh/meshModel.h"
#include "../vulkan/cubemap.h"
#include "../pipeline/meshRender.h"
#include "../pipeline/shadowCubemap.h"
#include "../vulkan/command.h"

inline static uint32_t shadowCubemapSize = 1024;

class ShadowCubemap : public vkn::Cubemap
{
    ShadowCubemapPushConstants pushConstants_;
    vkn::Image depthImage_;

    void UpdateCubemapFace(uint32_t faceIndex, vk::CommandBuffer& commandBuffer, int lightIndex, std::vector<LightData>& lights, std::vector<MeshModel>& meshes);
    void CreateFramebuffer(vk::CommandBuffer& commandBuffer);
    void CreateDepthImage(vk::CommandBuffer& commandBuffer);

public:
    void CreateShadowMap(vk::CommandBuffer& commandBuffer);
    void DrawShadowMap(vk::CommandBuffer& commandBuffer, int lightIndex, std::vector<LightData>& lights, std::vector<MeshModel>& meshes);
};

#endif
