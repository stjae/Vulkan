#ifndef IMGUI_H
#define IMGUI_H

#include "common.h"
#include "camera.h"
#include "scene.h"
#include "viewport.h"
#include "../imgui/ImGuizmo.h"

class MyImGui
{
    Descriptor descriptor_;
    std::vector<DescriptorSetLayoutData> descriptorSetLayoutData_;
    std::vector<vk::DescriptorSet> viewportDescriptorSets_;

public:
    void Setup(const vk::RenderPass& renderPass, Viewport& viewport);

    void Draw(Scene& scene, Viewport& viewport, size_t frameIndex);
    void DrawDockSpace(Scene& scene);
    void DrawViewport(Scene& scene, Viewport& viewport, size_t frameIndex);
    void SetViewportUpToDate(Viewport& viewport, const ImVec2& viewportPanelSize);
    void DrawImGuizmo(Scene& scene, const ImVec2& viewportPanelPos);
    void DrawObjectWindow(Scene& scene);
    void DrawResourceWindow(Scene& scene);

    void ShowInformationOverlay(const Scene& scene);
    void RecreateViewportDescriptorSets(const Viewport& viewport);

    ~MyImGui();
};

#endif