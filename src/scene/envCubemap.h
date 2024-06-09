#ifndef ENVCUBEMAP_H
#define ENVCUBEMAP_H

#include "../common.h"
#include "mesh.h"
#include "../vulkan/device.h"
#include "../vulkan/cubemap.h"
#include "../vulkan/pipeline.h"
#include "../vulkan/command.h"

class EnvCubemap : public vkn::Cubemap
{
    CubemapPushConstants m_pushConstants;

    void CreateFramebuffer(const vkn::Pipeline& cubemapPipeline);
    void DrawEnvCubemapFace(uint32_t faceIndex, const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, const vk::CommandBuffer& commandBuffer);

public:
    void CreateEnvCubemap(uint32_t cubemapSize, vk::Format format, vk::ImageUsageFlags usage, const vkn::Pipeline& cubemapPipeline, const vk::CommandBuffer& commandBuffer);
    void DrawEnvCubemap(int size, const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, const vk::CommandBuffer& commandBuffer);
    void DrawMipmap(const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, const vk::CommandBuffer& commandBuffer);
};

#endif
