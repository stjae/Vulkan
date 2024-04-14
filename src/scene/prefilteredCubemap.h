#ifndef PREFILTEREDCUBEMAP_H
#define PREFILTEREDCUBEMAP_H

#include "../vulkan/pipeline.h"
#include "../vulkan/cubemap.h"
#include "mesh/meshPrimitive.h"

class PrefilteredCubemap : public vkn::Cubemap
{
    int numMips_;
    PrefilteredCubemapPushConstants pushConstants_;
    vk::Sampler mipmapSampler_;

    void CreateMipmap(vk::CommandBuffer& commandBuffer);
    void CreateFramebuffer(const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer);
    void UpdateDescriptorSets(const vkn::Pipeline& cubemapPipeline, const Image& envMap);
    void DrawPrefilteredCubemapFace(float roughness, uint32_t faceIndex, const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer);
    void Draw(uint32_t mipSize, float roughness, const Mesh& envCube, const vkn::Image& envMap, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer);

public:
    vkn::Image mipmap;
    vk::DescriptorImageInfo mipmapDescriptorImageInfo;

    void CreatePrefilteredCubemap(int numMips, uint32_t cubemapSize, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer);
    void DrawPrefilteredCubemap(const Mesh& envCube, const Image& envMap, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer);
    void CopyToMipmap(uint32_t mipSize, int mipLevel, vk::CommandBuffer& commandBuffer);
    void CreateMipmapSampler();
    ~PrefilteredCubemap();
};

#endif
