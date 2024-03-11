#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "../common.h"
#include "../vulkan/image.h"
#include "../vulkan/command.h"
#include "../vulkan/swapchain.h"
#include "../scene/scene.h"

struct PipelineState
{
    Pipeline meshRender;
    Pipeline shadowMap;
};

struct ShadowMapPass
{
    int32_t width, height;
    std::array<vk::Framebuffer, 6> framebuffers;
    Image depth;
    vk::RenderPass renderPass;
};

class Viewport
{
    vk::CommandPool commandPool_;
    vk::CommandBuffer commandBuffer_;

    PipelineState pipelineState_;
    vk::RenderPass viewportRenderPass_;

    Image colorPicked_;

    // shadowCubeMap
    uint32_t shadowMapSize_;
    vk::Format shadowMapImageFormat_;
    Image shadowCubeMap_;
    std::array<vk::ImageView, 6> shadowCubeMapFaceImageViews_;
    ShadowMapPass shadowMapPass_;
    void PrepareShadowCubeMap();
    void CreateShadowMapRenderPass();

    void CreateViewportRenderPass();
    void CreateViewportFrameBuffer();

public:
    vk::Framebuffer framebuffer;

    Image viewportImage;
    Image depthImage;
    Image colorID;

    vk::Extent2D extent;
    ImVec2 panelPos;
    ImVec2 panelSize;
    float panelRatio;
    bool outDated;

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
