#ifndef __IMGUI_H__
#define __IMGUI_H__

#include "common.h"
#include "engine.h"
#include "camera.h"
#include "scene.h"
#include <imguizmo/ImGuizmo.h>

class MyImGui
{
public:
    void Setup(const std::unique_ptr<GraphicsEngine>& engine);
    void Draw(Camera& camera, const std::unique_ptr<Scene>& scene);
};

#endif // __IMGUI_H__