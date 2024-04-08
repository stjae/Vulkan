#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "../common.h"
#include "light.h"
#include "mesh.h"
#include "../vulkan/cubemap.h"
#include "../vulkan/pipeline.h"
#include "../vulkan/command.h"

inline static uint32_t shadowCubemapSize = 1024;

class ShadowCubemap : public vkn::Cubemap
{
    vkn::Image depthImage;

    void UpdateCubemapFace(uint32_t faceIndex, vk::CommandBuffer& commandBuffer, int lightIndex, std::vector<LightData>& lights, std::vector<Mesh>& meshes);
    void CreateFramebuffer(vk::CommandBuffer& commandBuffer);
    void CreateDepthImage(vk::CommandBuffer& commandBuffer);

public:
    void CreateShadowMap(vk::CommandBuffer& commandBuffer);
    void DrawShadowMap(vk::CommandBuffer& commandBuffer, int lightIndex, std::vector<LightData>& lights, std::vector<Mesh>& meshes);
};

#endif
