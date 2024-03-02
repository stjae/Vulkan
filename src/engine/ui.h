#ifndef IMGUI_H
#define IMGUI_H

#include "../common.h"
#include "../scene/camera.h"
#include "../scene/scene.h"
#include "../scene/light.h"
#include "viewport.h"
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_glfw.h"
#include "../../imgui/imgui_impl_vulkan.h"
#if defined(_WIN32)
#include "../../imgui/imgui_impl_win32.h"
#endif
#include "../../imgui/ImGuizmo.h"
#include "../../imgui/imgui_internal.h"

class UI
{
    inline static vk::DescriptorPool descriptorPool_;
    inline static std::vector<DescriptorBinding> descriptorBindings_;
    inline static std::vector<vk::DescriptorSetLayout> descriptorSetLayouts_;
    inline static std::vector<vk::DescriptorSet> descriptorSets_;

    static void DrawDockSpace(Scene& scene);
    static void DrawViewport(Scene& scene, Viewport& viewport, size_t frameIndex);
    static void SetViewportUpToDate(Viewport& viewport, const ImVec2& viewportPanelSize);
    static void DrawImGuizmo(Scene& scene, const ImVec2& viewportPanelPos);
    static void DrawObjectWindow(Scene& scene);
    static void DrawResourceWindow(Scene& scene);
    static void ShowInformationOverlay(const Scene& scene);

public:
    inline static ImDrawData* imDrawData;

    static void Setup(const vk::RenderPass& renderPass, Viewport& viewport);
    static void Draw(Scene& scene, Viewport& viewport, size_t frameIndex);
    static void RecreateViewportDescriptorSets(const Viewport& viewport);
    ~UI();
};

#endif