// Update handling for click, drawing, and resizing in the viewport
// ビューポートでのクリック・描画・リサイズ対応更新

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "../common.h"
#include "../scene/scene.h"
#include "../vulkan/image.h"
#include "../vulkan/command.h"
#include "../vulkan/swapchain.h"
#include "../pipeline/meshRenderPipeline.h"
#include "../pipeline/postProcessPipeline.h"
#include "../pipeline/colorIDPipeline.h"
#include "../pipeline/shadowMapPipeline.h"
#include "../pipeline/shadowCubemapPipeline.h"
#include "../pipeline/envCubemapPipeline.h"
#include "../pipeline/irradianceCubemapPipeline.h"
#include "../pipeline/prefilteredCubemapPipeline.h"
#include "../pipeline/brdfLutPipeline.h"
#include "../pipeline/skyboxRenderPipeline.h"
#include "../pipeline/lineRenderPipeline.h"
#include "../pipeline/physicsDebugPipeline.h"
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
    void PickColor(const vk::CommandBuffer& commandBuffer, double mouseX, double mouseY, Scene& scene);
    void Draw(const vk::CommandBuffer& commandBuffer, const Scene& scene);
    ~Viewport();
    void UpdateImage();
    static float GetRatio() { return s_panelRatio; }
};

#endif
