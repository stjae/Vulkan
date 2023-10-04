#ifndef _BASEAPP_H_
#define _BASEAPP_H_

#include "common.h"

class Application
{
public:
    Application(const int width, const int height, const char* wName, bool debugMode);

    void run();

private:
    void InitVulkan();
    void MainLoop();

    Window m_window;
    Instance m_instance;
    Log m_log;
};

#endif