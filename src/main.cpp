#include "engine/baseApp.h"
#include "common.h"

int main()
{
    std::unique_ptr<Application> app = std::make_unique<Application>(1920, 1080, "Vulkan");

    app->Run();

    return 0;
}