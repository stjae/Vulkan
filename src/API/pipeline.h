#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include "../common.h"
#include "swapchain.h"
#include "shader.h"
#include "framebuffer.h"

struct PipelineInDetails {
    std::string vertexShaderFilepath;
    std::string fragmentShaderFilepath;
    vk::Extent2D swapchainExtent;
    vk::Format swapchainImageFormat;
};

struct PipelineOutDetails {
    vk::Pipeline graphicsPipeline;
    vk::PipelineLayout pipelineLayout;
    vk::RenderPass renderPass;
};

class GraphicsPipeline
{
public:
    ~GraphicsPipeline();
    void CreatePipeline();
    vk::PipelineLayout CreatePipelineLayout();
    vk::RenderPass CreateRenderPass();

    static PipelineInDetails input;
    static PipelineOutDetails output;

private:
    Shader m_shader;
};

#endif