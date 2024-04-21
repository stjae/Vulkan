#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "../vulkan/image.h"
#include "../scene/mesh/meshModel.h"
#include "../pipeline/shadowMap.h"

inline static uint32_t shadowMapSize = 2048;

class ShadowMap : public vkn::Image
{
    ShadowMapPushConstants pushConstants_;

    void CreateFramebuffer(vk::CommandBuffer& commandBuffer);

public:
    void CreateShadowMap(vk::CommandBuffer& commandBuffer);
    void DrawShadowMap(vk::CommandBuffer& commandBuffer, std::vector<MeshModel>& meshes);
};

#endif