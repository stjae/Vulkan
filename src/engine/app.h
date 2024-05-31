#ifndef BASEAPP_H
#define BASEAPP_H

#include "engine.h"
#include "../time.h"
#include "../common.h"

class Application
{
    Window m_window;
    Engine m_engine;

public:
    Application(int width, int height, const char* title) : m_window(width, height, title) {}

    void Run();
};

#endif