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
#include "../imgui/imgui_impl_win32.h"
#endif
#include "../../imgui/ImGuizmo.h"
#include "../../imgui/imgui_internal.h"

class MyImGui
{
    vk::DescriptorPool descriptorPool_;
    std::vector<DescriptorBinding> descriptorBindings_;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts_;
    std::vector<vk::DescriptorSet> descriptorSets_;

    void DrawDockSpace(Scene& scene);
    void DrawViewport(Scene& scene, Viewport& viewport, size_t frameIndex);
    void SetViewportUpToDate(Viewport& viewport, const ImVec2& viewportPanelSize);
    void DrawImGuizmo(Scene& scene, const ImVec2& viewportPanelPos);
    void DrawObjectWindow(Scene& scene);
    void DrawResourceWindow(Scene& scene);
    void ShowInformationOverlay(const Scene& scene);

public:
    void Setup(const vk::RenderPass& renderPass, Viewport& viewport);
    void Draw(Scene& scene, Viewport& viewport, size_t frameIndex);
    void RecreateViewportDescriptorSets(const Viewport& viewport);

    ~MyImGui();
};

#endif