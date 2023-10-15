#include "baseApp.h"
#include "common.h"

int main()
{
    Application* app = new Application(800, 600, "Vulkan");

    app->Run();

    delete app;

    return 0;
}