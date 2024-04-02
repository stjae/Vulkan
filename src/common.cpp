#include "common.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

std::string& GetFrameRate()
{
    static int frameCount;
    static double delta, currentTime, lastTime;
    static std::string frameRate;

    currentTime = glfwGetTime();
    delta = currentTime - lastTime;

    if (delta > 1.0) {
        std::stringstream title;
        title << frameCount << " fps, " << 1000.0f / (float)frameCount << " ms";

        frameRate = title.str();

        lastTime = currentTime;
        frameCount = 0;
    }

    frameCount++;

    return frameRate;
}

std::vector<char> FetchCode(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        spdlog::error("failed to open file: {}", filename);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), (std::streamsize)fileSize);

    file.close();

    if (!buffer.empty()) {
        Log(debugMode, fmt::terminal_color::white, "{0} loaded with code length of {1}",
            filename, buffer.size());
    }

    return buffer;
}

std::string LaunchNfd(nfdfilteritem_t filterItem)
{
    NFD_Init();
    std::string filePath;

    nfdchar_t* outPath;
    nfdresult_t result = NFD_OpenDialog(&outPath, &filterItem, 1, nullptr);
    if (result == NFD_OKAY) {
        puts("Success!");
        puts(outPath);
        filePath = outPath;
        NFD_FreePath(outPath);
    } else if (result == NFD_CANCEL) {
        puts("User pressed cancel.");
    } else {
        printf("Error: %s\n", NFD_GetError());
    }

    NFD_Quit();
    return filePath;
}

void* AlignedAlloc(size_t dynamicBufferAlignment, size_t bufferSize)
{
#ifdef __APPLE__
    return aligned_alloc(dynamicBufferAlignment, bufferSize);
#elif _WIN32
    return _aligned_malloc(bufferSize, dynamicBufferAlignment);
#else
    return nullptr;
#endif
}

void AlignedFree(void* aligned)
{
#ifdef __APPLE__
    free(aligned);
#elif _WIN32
    _aligned_free(aligned);
#endif
}

void vkn::CheckResult(vk::Result result)
{
    if (result != vk::Result::eSuccess) {
        Log(debugMode, fmt::terminal_color::red, "VkResult: ", vk::to_string(result), __FILE__, __LINE__);
        assert(result == vk::Result::eSuccess);
    }
}
