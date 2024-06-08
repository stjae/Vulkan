#ifndef PREFILTEREDCUBEMAP_H
#define PREFILTEREDCUBEMAP_H

#include "../vulkan/pipeline.h"
#include "../vulkan/cubemap.h"
#include "mesh.h"

class PrefilteredCubemap : public vkn::Cubemap
{
    int m_numMips;
    uint32_t m_mipSize;
    PrefilteredCubemapPushConstants m_pushConstants;
    vk::Sampler m_mipmapSampler;

    void CreateMipmap(vk::CommandBuffer& commandBuffer);
    void CreateMipmapSampler();
    void CreateFramebuffer(const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer);
    void CopyToMipmap(uint32_t mipSize, int mipLevel, vk::CommandBuffer& commandBuffer);
    void DrawPrefilteredCubemapFace(float roughness, uint32_t faceIndex, const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer);
    void Draw(uint32_t mipSize, float roughness, const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer);

public:
    vkn::Image m_mipmap;
    vk::DescriptorImageInfo m_mipmapDescriptorImageInfo;

    void CreatePrefilteredCubemap(int numMips, uint32_t cubemapSize, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer);
    void DrawPrefilteredCubemap(const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer);
    ~PrefilteredCubemap();
};

#endif
