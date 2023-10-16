#ifndef _BASEAPP_H_
#define _BASEAPP_H_

#include "engine.h"
#include "common.h"
#include "scene.h"

class Application
{
public:
    Application(const int width, const int height, const char* wName);
    ~Application();

    void Run();
    void GetFramerate();

private:
    Window m_window;
    GraphicsEngine* m_engine;
    Scene* m_scene;
};

#endif