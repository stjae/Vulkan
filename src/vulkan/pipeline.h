#ifndef PIPELINE_H
#define PIPELINE_H

#include "../common.h"
#include "shader.h"
#include "descriptor.h"
#include "image.h"
#include "../scene/meshData.h"

inline vk::Format shadowMapImageFormat = vk::Format::eR32Sfloat;
inline vk::Format shadowMapDepthFormat = vk::Format::eD32Sfloat;

struct MeshRenderPushConstants
{
    int meshIndex;
    int materialID;
} inline meshRenderPushConsts;

struct ShadowMapPushConstants
{
    glm::mat4 view;
    glm::vec2 padding;
    int meshIndex;
    int lightIndex;
} inline shadowMapPushConsts;

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
    std::vector<vk::DescriptorBufferInfo> meshDescriptors;
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
    std::vector<vk::DescriptorBufferInfo> meshDescriptors;
} inline shadowMapPipeline;

void CreatePipeline();
void SetUpDescriptors();
void CreateMeshRenderPass();
void CreateShadowMapRenderPass();

#endif