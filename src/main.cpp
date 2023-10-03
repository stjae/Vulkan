#include "baseApp.h"
#include "common.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

int main()
{
    HelloTriangleApplication app(800, 600, "Vulkan");

    try {
        app.run();
    } catch (const std::exception& e) {
        spdlog::error(e.what());
        return 1;
    }

    return 0;
}