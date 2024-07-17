#ifndef PIPELINE_H
#define PIPELINE_H

#include "../common.h"
#include "shader.h"
#include "descriptor.h"
#include "../scene/meshBase.h"

inline vk::Format shadowMapImageFormat = vk::Format::eR32Sfloat;
inline vk::Format shadowMapDepthFormat = vk::Format::eD32Sfloat;

namespace vkn {

class Pipeline
{
protected:
    vk::VertexInputBindingDescription m_bindingDesc;
    std::array<vk::VertexInputAttributeDescription, 5> m_vertexInputAttribDesc;
    vk::PipelineVertexInputStateCreateInfo m_vertexInputStateCI;
    vk::PipelineInputAssemblyStateCreateInfo m_inputAssemblyStateCI;
    std::array<vk::DynamicState, 2> m_dynamicStates;
    vk::PipelineDynamicStateCreateInfo m_dynamicStateCI;
    vk::PipelineViewportStateCreateInfo m_viewportStateCI;
    vk::PipelineRasterizationStateCreateInfo m_rasterizeStateCI;
    vk::PipelineMultisampleStateCreateInfo m_multisampleStateCI;
    vk::PipelineDepthStencilStateCreateInfo m_depthStencilStateCI;
    vk::PipelineColorBlendStateCreateInfo m_colorBlendStateCI;
    vk::GraphicsPipelineCreateInfo m_pipelineCI;

    virtual void CreatePipeline() = 0;
    virtual void CreateRenderPass() = 0;
    virtual void SetUpDescriptors() = 0;

public:
    vk::Pipeline m_pipeline;
    vk::PipelineLayout m_pipelineLayout;
    vk::RenderPass m_renderPass;
    vkn::Shader m_shaderModule;
    std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;
    vk::DescriptorPool m_descriptorPool;
    std::vector<vk::DescriptorSet> m_descriptorSets;

    Pipeline();
    void Destroy();
};

} // namespace vkn

struct PhysicsDebugPushConstants
{
    int meshIndex;
} inline physicsDebugPushConstants;

struct MeshRenderPushConstants
{
    int meshIndex;
    int materialIndex;
    int lightCount;
    float iblExposure;
} inline meshRenderPushConsts;

struct SkyboxRenderPushConstants
{
    float exposure;
} inline skyboxRenderPushConstants;

struct ShadowMapPushConstants
{
    glm::mat4 viewProj;
    int meshIndex;
};

struct ShadowCubemapPushConstants
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
    glm::mat4 viewProj;
    float roughness;
};

struct PostProcessPushConstants
{
    int useMotionBlur = 0;
    float divisor = 20.0f;
} inline postProcessPushConstants;

#endif