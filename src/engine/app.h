#ifndef BASEAPP_H
#define BASEAPP_H

#include "engine.h"
#include "../common.h"

class Application
{
    Window window_;
    Engine engine_;

public:
    Application(int width, int height, const char* wName) : window_(width, height, wName) {}

    void Run();
};

#endif