#include "baseApp.h"
#include "common.h"

#ifdef NDEBUG
const bool debugMode = false;
#else
const bool debugMode = true;
#endif

int main()
{
    Application app(800, 600, "Vulkan", debugMode);

    try {
        app.run();
    } catch (const std::exception& e) {
        spdlog::error(e.what());
        return 1;
    }

    return 0;
}