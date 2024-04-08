#ifndef PIPELINE_H
#define PIPELINE_H

#include "../common.h"
#include "shader.h"
#include "descriptor.h"
#include "image.h"
#include "../scene/meshBase.h"

inline vk::Format shadowMapImageFormat = vk::Format::eR32Sfloat;
inline vk::Format shadowMapDepthFormat = vk::Format::eD32Sfloat;

struct MeshRenderPushConstants
{
    int meshIndex;
    int materialID;
    int lightCount;
    int useIBL;
} inline meshRenderPushConsts;

struct ShadowMapPushConstants
{
    glm::mat4 view;
    glm::vec2 padding;
    int meshIndex;
    int lightIndex;
} inline shadowMapPushConsts;

struct GenEnvCubePushConstants
{
    glm::mat4 view;
    glm::mat4 proj;
} inline genEnvCubePushConstants;

namespace vkn {
class Pipeline
{
protected:
    vk::VertexInputBindingDescription bindingDesc_;
    std::array<vk::VertexInputAttributeDescription, 5> vertexInputAttribDesc_;
    vk::PipelineVertexInputStateCreateInfo vertexInputStateCI_;
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCI_;
    std::array<vk::DynamicState, 2> dynamicStates_;
    vk::PipelineDynamicStateCreateInfo dynamicStateCI_;
    vk::PipelineViewportStateCreateInfo viewportStateCI_;
    vk::PipelineRasterizationStateCreateInfo rasterizeStateCI_;
    vk::PipelineMultisampleStateCreateInfo multisampleStateCI_;
    vk::PipelineDepthStencilStateCreateInfo depthStencilStateCI_;
    vk::PipelineColorBlendStateCreateInfo colorBlendStateCI_;

    vk::GraphicsPipelineCreateInfo pipelineCI_;

    virtual void CreatePipeline() = 0;
    virtual void CreateRenderPass() = 0;
    virtual void SetUpDescriptors() = 0;

public:
    vk::Pipeline pipeline;
    vk::PipelineLayout pipelineLayout;
    vk::RenderPass renderPass;
    vkn::Shader shader;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;

    Pipeline();
    ~Pipeline();
};

class MeshRenderPipeline : public Pipeline
{
public:
    vk::DescriptorBufferInfo cameraDescriptor;
    vk::DescriptorBufferInfo lightDescriptor;
    std::vector<vk::DescriptorBufferInfo> meshDescriptors;
    std::vector<vk::DescriptorImageInfo> shadowCubeMapDescriptors;

    void CreatePipeline() override;
    void SetUpDescriptors() override;
    void CreateRenderPass() override;
} inline meshRenderPipeline;

class ShadowMapPipeline : public Pipeline
{
public:
    vk::DescriptorBufferInfo cameraDescriptor;
    vk::DescriptorBufferInfo lightDescriptor;
    std::vector<vk::DescriptorBufferInfo> meshDescriptors;

    void CreatePipeline() override;
    void SetUpDescriptors() override;
    void CreateRenderPass() override;
} inline shadowMapPipeline;

class EnvTexPipeline : public Pipeline
{
public:
    void CreatePipeline() override;
    void SetUpDescriptors() override;
    void CreateRenderPass() override;
} inline envTexPipeline;

class IrradianceCubemapPipeline : public Pipeline
{
public:
    void CreatePipeline() override;
    void SetUpDescriptors() override;
    void CreateRenderPass() override;
} inline irradianceCubemapPipeline;

class SkyboxRenderPipeline : public Pipeline
{
public:
    void CreatePipeline() override;
    void SetUpDescriptors() override;
    void CreateRenderPass() override;
} inline skyboxRenderPipeline;

} // namespace vkn

#endif