#ifndef _BASEAPP_H_
#define _BASEAPP_H_

#include "engine.h"
#include "common.h"

class Application
{
public:
    Application(const int width, const int height, const char* wName);
    ~Application();

    void run();

private:
    Window m_window;
    GraphicsEngine* engine;
};

#endif