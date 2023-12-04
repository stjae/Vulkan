#ifndef __BASEAPP_H__
#define __BASEAPP_H__

#include "engine.h"
#include "common.h"
#include "scene.h"
#include "camera.h"
#include "imgui.h"

class Application
{
public:
    inline static double delta;

    Application(const int width, const int height, const char* wName);

    void Run();
    void GetFramerate();

private:
    Window window;
    std::unique_ptr<GraphicsEngine> engine;
    std::unique_ptr<Scene> scene;

    MyImGui imgui;
};

#endif // __BASEAPP_H__