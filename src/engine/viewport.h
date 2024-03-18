#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "../common.h"
#include "../vulkan/image.h"
#include "../vulkan/command.h"
#include "../vulkan/swapchain.h"
#include "../vulkan/pipeline.h"
#include "../scene/scene.h"

class Viewport
{
    vk::CommandPool commandPool_;
    vk::CommandBuffer commandBuffer_;

    Image colorPicked_;

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
    void CreateViewportFrameBuffer();
    const int32_t* PickColor(size_t frameIndex, double mouseX, double mouseY);
    void Draw(size_t frameIndex, Scene& scene);
    ~Viewport();
};

#endif
