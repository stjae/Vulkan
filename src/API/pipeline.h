#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include "../common.h"
#include "swapchain.h"
#include "shader.h"

class GraphicsPipeline
{
public:
    ~GraphicsPipeline();
    void CreatePipeline();
    vk::PipelineLayout CreatePipelineLayout();
    vk::RenderPass CreateRenderPass();

private:
    Shader m_shader;
};

#endif