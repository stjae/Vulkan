#include "baseApp.h"

Application::Application(const int width, const int height, const char* wName)
    : m_window(width, height, wName)
{
    engine = new GraphicsEngine(m_window.width, m_window.height, m_window.window);
}

void Application::run()
{
    while (!glfwWindowShouldClose(m_window.window)) {
        glfwPollEvents();
        engine->Render();
    }
}

Application::~Application()
{
    delete engine;
}