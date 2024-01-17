#ifndef BASEAPP_H
#define BASEAPP_H

#include "engine.h"
#include "common.h"
#include "scene.h"
#include "camera.h"

class Application
{
public:
    Application(const int width, const int height, const char* wName);

    void Run();
    static std::string& GetFramerate();

private:
    Window window_;
    std::unique_ptr<Engine> engine_;
    std::unique_ptr<Scene> scene_;
};

#endif