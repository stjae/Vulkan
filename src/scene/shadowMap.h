#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "../common.h"
#include "light.h"
#include "../vulkan/pipeline.h"
#include "../vulkan/command.h"

inline static uint32_t shadowMapSize = 1024;

class ShadowMap
{
    std::array<vk::ImageView, 6> shadowCubeMapFaceImageViews;
    std::array<vk::Framebuffer, 6> framebuffers;
    vkn::Image shadowCubeMap;
    vkn::Image depth;

    void UpdateCubeMapFace(uint32_t faceIndex, vk::CommandBuffer& commandBuffer, int lightIndex, std::vector<LightData>& lights, std::vector<Mesh>& meshes);
    void CreateFrameBuffer(vk::CommandBuffer& commandBuffer);

public:
    void PrepareShadowCubeMap(vk::CommandBuffer& commandBuffer);
    void DrawShadowMap(vk::CommandBuffer& commandBuffer, int lightIndex, std::vector<LightData>& lights, std::vector<Mesh>& meshes);
    ~ShadowMap();
};

#endif
