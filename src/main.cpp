#include "common.h"
#include "baseApp.h"

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

int main() {
    HelloTriangleApplication app(800, 600, enableValidationLayers);

    try {
        app.run();
    } catch (const std::exception& e) {
        SPDLOG_INFO(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}