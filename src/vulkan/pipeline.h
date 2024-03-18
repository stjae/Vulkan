#ifndef PIPELINE_H
#define PIPELINE_H

#include "../common.h"
#include "shader.h"
#include "descriptor.h"
#include "image.h"
#include "../scene/meshData.h"

inline vk::Format shadowMapImageFormat = vk::Format::eR32Sfloat;
inline vk::Format shadowMapDepthFormat = vk::Format::eD32Sfloat;

struct MeshRenderPipeline
{
    vk::Pipeline pipeline;
    vk::PipelineLayout pipelineLayout;
    vk::RenderPass renderPass;
    Shader shader;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;
    vk::DescriptorBufferInfo cameraDescriptor;
    vk::DescriptorBufferInfo lightDescriptor;
    vk::DescriptorBufferInfo meshDescriptor;
    std::vector<vk::DescriptorImageInfo> shadowCubeMapDescriptors;
} inline meshRenderPipeline;

struct ShadowMapPipeline
{
    vk::Pipeline pipeline;
    vk::PipelineLayout pipelineLayout;
    vk::RenderPass renderPass;
    Shader shader;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;
    vk::DescriptorBufferInfo cameraDescriptor;
    vk::DescriptorBufferInfo lightDescriptor;
    vk::DescriptorBufferInfo meshDescriptor;
} inline shadowMapPipeline;

void CreatePipeline();
void CreateDescriptorSetLayouts();
void CreateMeshRenderPass();
void CreateShadowMapRenderPass();

#endif