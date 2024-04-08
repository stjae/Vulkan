#ifndef ENVCUBEMAP_H
#define ENVCUBEMAP_H

#include "../common.h"
#include "../scene/mesh.h"
#include "../vulkan/device.h"
#include "../vulkan/cubemap.h"
#include "../vulkan/pipeline.h"
#include "../vulkan/command.h"

class EnvCubemap : public vkn::Cubemap
{
    uint32_t cubemapSize_;

    void CreateFramebuffer(const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer);
    void DrawEnvCubemapFace(uint32_t faceIndex, const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer);
    void UpdateDescriptorSets(const vkn::Pipeline& cubemapPipeline, const vkn::Image& envMap);

public:
    void CreateEnvCubemap(uint32_t cubemapSize, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer);
    void DrawEnvCubemap(const Mesh& envCube, const vkn::Image& envMap, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer);
};

#endif
