#ifndef __BASEAPP_H__
#define __BASEAPP_H__

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
    std::unique_ptr<GraphicsEngine> engine_;
    std::shared_ptr<Scene> scene_;
};

#endif // __BASEAPP_H__