#include "baseApp.h"
#include "common.h"

int main()
{
    std::unique_ptr<Application> app = std::make_unique<Application>(800, 600, "Vulkan");

    app->Run();

    return 0;
}