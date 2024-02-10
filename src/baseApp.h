#ifndef BASEAPP_H
#define BASEAPP_H

#include "engine.h"
#include "common.h"
#include "scene/scene.h"
#include "scene/camera.h"

class Application
{
public:
    Application(int width, int height, const char* wName);

    void Run();

private:
    Window window_;
    Engine engine_;
};

#endif