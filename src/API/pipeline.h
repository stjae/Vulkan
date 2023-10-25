#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include "../common.h"
#include "swapchain.h"
#include "shader.h"
#include "../structs.h"
#include "../mesh.h"
#include "descriptor.h"
#include "../scene.h"

class GraphicsPipeline
{
public:
    GraphicsPipeline(const vk::Device& vkDevice, const SwapchainDetail& swapchainDetail, std::unique_ptr<Scene>& scene)
        : vkDevice(vkDevice), swapchainDetail(swapchainDetail), shader(vkDevice), descriptor(vkDevice), scene(scene) {}
    void CreatePipeline();
    vk::PipelineLayout CreatePipelineLayout();
    vk::RenderPass CreateRenderPass();
    void CreateDescriptorPool();
    void AllocateDescriptorSet(vk::DescriptorSet& descriptorSet);
    ~GraphicsPipeline();

    vk::Pipeline vkPipeline;
    vk::PipelineLayout vkPipelineLayout;
    vk::RenderPass vkRenderPass;
    std::unique_ptr<Scene>& scene;
    const SwapchainDetail& swapchainDetail;

private:
    const vk::Device& vkDevice;
    Shader shader;
    Descriptor descriptor;
};

#endif