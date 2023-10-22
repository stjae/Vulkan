#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include "../common.h"
#include "swapchain.h"
#include "shader.h"
#include "../structs.h"
#include "../mesh.h"

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

inline vk::Pipeline graphicsPipeline;
inline vk::PipelineLayout pipelineLayout;
inline vk::RenderPass renderPass;

#endif