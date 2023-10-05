#include "baseApp.h"
#include "common.h"


int main()
{
    Application app(800, 600, "Vulkan");

    try {
        app.run();
    } catch (const std::exception& e) {
        spdlog::error(e.what());
        return 1;
    }

    return 0;
}