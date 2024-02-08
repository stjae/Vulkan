#include "baseApp.h"
#include "common.h"

int main()
{
    std::unique_ptr<Application> app = std::make_unique<Application>(1024, 720, "Vulkan");

    app->Run();

    return 0;
}