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
    void DrawImGuizmo(std::unique_ptr<Scene>& scene, int currentItem);
    void DrawDockSpace(std::unique_ptr<Scene>& scene, Viewport& viewport, size_t frameIndex);
    void RecreateViewportDescriptorSets(Viewport& viewport);
    void SetCameraControl(std::unique_ptr<Scene>& scene);
    void Draw(std::unique_ptr<Scene>& scene, Viewport& viewport, size_t frameIndex);
    ~MyImGui();
};

#endif