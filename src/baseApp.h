#ifndef _BASEAPP_H_
#define _BASEAPP_H_

#include "engine.h"
#include "common.h"
#include "scene.h"
#include "camera.h"

class Application
{
public:
    inline static double delta;

    Application(const int width, const int height, const char* wName);

    void SetupImGui();
    void Run();
    void GetFramerate();

private:
    Window window;
    std::unique_ptr<GraphicsEngine> engine;
    std::unique_ptr<Scene> scene;

    Camera camera;
};

#endif