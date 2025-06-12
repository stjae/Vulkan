#ifndef IMGUI_H
#define IMGUI_H

#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/imgui_internal.h"
#include "ImGuizmo/ImGuizmo.h"

#include "../common.h"
#include "../scene/camera.h"
#include "../scene/scene.h"
#include "viewport.h"
#include "sceneSerializer.h"
#include "../engine/script/script.h"
#include "../myTime.h"
#include <typeinfo>

class UI
{
    vk::DescriptorPool m_descriptorPool;
    std::vector<vkn::DescriptorBinding> m_descriptorBindings;
    std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;
    vk::DescriptorSet m_viewportImageDescriptorSet;

    vkn::Image m_plusIcon;
    vkn::Image m_lightIcon;
    vkn::Image m_cameraIcon;
    vkn::Image m_noCameraIcon;
    vkn::Image m_cubeIcon;
    vkn::Image m_playIcon;
    vkn::Image m_stopIcon;
    vk::DescriptorSet m_plusIconDescriptorSet;
    vk::DescriptorSet m_lightIconDescriptorSet;
    vk::DescriptorSet m_cameraIconDescriptorSet;
    vk::DescriptorSet m_noCameraIconDescriptorSet;
    vk::DescriptorSet m_cubeIconDescriptorSet;
    vk::DescriptorSet m_playIconDescriptorSet;
    vk::DescriptorSet m_stopIconDescriptorSet;
    // TODO:
    // vk::DescriptorSet m_hdriThumbnailDescriptorSet;
    // vk::DescriptorSet m_shadowMapDescriptorSet;

    void DrawDockSpace(const vk::CommandBuffer& commandBuffer, Scene& scene, bool& init);
    void DrawViewport(Scene& scene, Viewport& viewport);
    void DrawMeshGuizmo(Scene& scene, const Viewport& viewport);
    void DrawLightGuizmo(Scene& scene, const Viewport& viewport);
    void DrawSceneAttribWindow(const vk::CommandBuffer& commandBuffer, Scene& scene);
    void DrawResourceWindow(const vk::CommandBuffer& commandBuffer, Scene& scene);
    void ShowInformationOverlay();
    void DrawLightIcon(const Scene& scene, const Viewport& viewport);
    void DrawCameraIcon(const Scene& scene, const Viewport& viewport);

    static float GetButtonPadding(float desiredButtonSize, float paddingRatio) { return desiredButtonSize * paddingRatio; }
    static float GetIconSize(float desiredButtonSize, float padding) { return desiredButtonSize - padding * 2.0f; }

public:
    inline static bool s_dragDropped;
    inline static double s_dragDropMouseX;
    inline static double s_dragDropMouseY;
    inline static std::unique_ptr<Resource> s_dragDropResource;

    void Init(const vk::CommandBuffer& commandBuffer, const vk::RenderPass& renderPass);
    void Draw(const vk::CommandBuffer& commandBuffer, Scene& scene, Viewport& viewport, bool& init);
    void RecreateViewportDescriptorSet(const Viewport& viewport);
    void AcceptDragDrop(Scene& scene);
    void DrawInitPopup(const vk::CommandBuffer& commandBuffer, bool& init, Scene& scene);
    ~UI();
};

#endif