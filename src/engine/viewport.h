#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "../common.h"
#include "../scene/scene.h"
#include "../vulkan/vulkanImage.h"
#include "../vulkan/command.h"
#include "../vulkan/swapchain.h"
#include "../pipeline/meshRender.h"
#include "../pipeline/postProcess.h"
#include "../pipeline/colorID.h"
#include "../pipeline/shadowMapPipeline.h"
#include "../pipeline/shadowCubemapPipeline.h"
#include "../pipeline/envCubemapPipeline.h"
#include "../pipeline/irradianceCubemap.h"
#include "../pipeline/prefilteredCubemapPipeline.h"
#include "../pipeline/brdfLut.h"
#include "../pipeline/skyboxRender.h"
#include "../pipeline/lineRender.h"
#include "../pipeline/physicsDebug.h"
#include "../scene/mesh.h"

class Viewport
{
    friend class UI;
    friend class Engine;

    vk::Framebuffer m_framebuffer;
    vkn::Image m_imageSampled;
    vkn::Image m_imageResolved;
    vkn::Image m_depthSampled;
    vkn::Image m_imageFinal;

    vk::Framebuffer m_colorIDFramebuffer;
    vkn::Image m_colorID;
    vkn::Image m_depth;

    vk::CommandPool m_commandPool;
    vk::CommandBuffer m_commandBuffer;
    vk::CommandBuffer m_pickColorCommandBuffer;

    vkn::Image m_pickedColor;
    ImVec2 m_panelPos;
    ImVec2 m_panelSize;
    vk::Extent2D m_extent;
    inline static float s_panelRatio;
    bool m_outDated;
    bool m_isMouseHovered;

public:
    Viewport();
    void CreateImage();
    void DestroyImage();
    void PickColor(double mouseX, double mouseY, Scene& scene);
    void Draw(const Scene& scene);
    ~Viewport();
    void UpdateImage();
    static float GetRatio() { return s_panelRatio; }
};

#endif
