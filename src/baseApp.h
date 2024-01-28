#ifndef BASEAPP_H
#define BASEAPP_H

#include "engine.h"
#include "common.h"
#include "scene.h"
#include "camera.h"

class Application
{
public:
    Application(int width, int height, const char* wName);

    void Run();
    static std::string& GetFramerate();

private:
    Window window_;
    Engine engine_;
};

#endif