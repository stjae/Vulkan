#ifndef IMGUI_H
#define IMGUI_H

#include "common.h"
#include "camera.h"
#include "scene.h"
#include "../imgui/ImGuizmo.h"

class MyImGui
{
    vk::DescriptorPool imGuiDescriptorPool;

public:
    void Setup(GraphicsPipeline& pipeline);
    void DrawImGuizmo(std::unique_ptr<Scene>& scene, int currentItem);
    void DrawDockSpace(std::unique_ptr<Scene>& scene);
    void SetCameraControl(std::unique_ptr<Scene>& scene);
    void Draw(std::unique_ptr<Scene>& scene, int frameIndex);
    ~MyImGui();
};

#endif