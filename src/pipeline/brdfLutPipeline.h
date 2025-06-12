#ifndef BRDFLUTPIPELINE_H
#define BRDFLUTPIPELINE_H

#include "../vulkan/pipeline.h"

class BrdfLutPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override {}
    void CreateRenderPass() override;

public:
    void CreatePipeline() override;
} inline brdfLutPipeline;

#endif
