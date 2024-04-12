#ifndef PIPELINE_H
#define PIPELINE_H

#include "../common.h"
#include "shader.h"
#include "descriptor.h"
#include "../scene/meshBase.h"

inline vk::Format shadowMapImageFormat = vk::Format::eR32Sfloat;
inline vk::Format shadowMapDepthFormat = vk::Format::eD32Sfloat;

struct ShadowMapPushConstants
{
    glm::mat4 view;
    glm::vec2 padding;
    int meshIndex;
    int lightIndex;
};

struct CubemapPushConstants
{
    glm::mat4 view;
    glm::mat4 proj;
};

struct PrefilteredCubemapPushConstants
{
    glm::mat4 view;
    glm::mat4 proj;
    float roughness;
};

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

} // namespace vkn

#endif