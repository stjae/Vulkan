#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "../common.h"
#include "../scene/scene.h"
#include "../vulkan/image.h"
#include "../vulkan/command.h"
#include "../vulkan/swapchain.h"
#include "../pipeline/meshRender.h"
#include "../pipeline/shadowMap.h"
#include "../pipeline/shadowCubemap.h"
#include "../pipeline/envCubemap.h"
#include "../pipeline/irradianceCubemap.h"
#include "../pipeline/brdfLut.h"
#include "../pipeline/skyboxRender.h"
#include "../pipeline/lineRender.h"
#include "../scene/mesh/mesh.h"

class Viewport
{
    vk::CommandPool commandPool_;
    vk::CommandBuffer commandBuffer_;

    vkn::Image colorPicked_;

public:
    vk::Framebuffer framebuffer;

    vkn::Image viewportImage;
    vkn::Image depthImage;
    vkn::Image colorID;

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
    const int32_t* PickColor(double mouseX, double mouseY);
    void Draw(const Scene& scene);
    ~Viewport();
};

#endif
