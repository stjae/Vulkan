#ifndef BASEAPP_H
#define BASEAPP_H

#include "engine.h"
#include "../common.h"
#include "../scene/scene.h"
#include "../scene/camera.h"

class Application
{
    Window window_;
    Engine engine_;

public:
    Application(int width, int height, const char* wName);

    void Run();
};

#endif