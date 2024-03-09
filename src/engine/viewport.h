#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "../common.h"
#include "../vulkan/image.h"
#include "../vulkan/command.h"
#include "../vulkan/swapchain.h"
#include "../scene/scene.h"

struct ViewportFrame
{
    vk::Framebuffer framebuffer;

    Image viewportImage;
    Image depthImage;
    Image colorID;

    vk::CommandPool commandPool;
    vk::CommandBuffer commandBuffer;
};

struct PipelineState
{
    Pipeline meshRender;
};

class Viewport
{
    PipelineState pipelineState_;
    vk::RenderPass viewportRenderPass_;

    void CreateRenderPass();
    void CreateFrameBuffer();

public:
    std::vector<ViewportFrame> frames;

    vk::Extent2D extent;
    ImVec2 panelPos;
    ImVec2 panelSize;
    float panelRatio;
    bool outDated;

    Image colorPicked_;
    bool isMouseHovered;

    Viewport();
    void CreateViewportImages();
    void DestroyViewportImages();
    const int32_t* PickColor(size_t frameIndex, double mouseX, double mouseY);
    void Draw(size_t frameIndex, Scene& scene);
    const PipelineState& GetPipelineState() { return pipelineState_; };
    ~Viewport();
};

#endif
