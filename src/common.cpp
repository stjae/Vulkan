#include "common.h"

std::string& GetFrameRate()
{
    static int frameCount;
    static double delta, currentTime, lastTime;
    static std::string frameRate;

    currentTime = glfwGetTime();
    delta = currentTime - lastTime;

    if (delta > 1.0) {
        std::stringstream title;
        title << frameCount << " fps, " << 1000.0f / frameCount << " ms";

        frameRate = title.str().c_str();

        lastTime = currentTime;
        frameCount = 0;
    }

    frameCount++;

    return frameRate;
}

std::vector<char> ReadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        spdlog::error("failed to open file: {}", filename);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    if (buffer.size() > 0) {
        Log(debug, fmt::terminal_color::white, "{0} loaded with code length of {1}",
            filename, buffer.size());
    }

    return buffer;
}

void* AlignedAlloc(size_t dynamicBufferAlignment, size_t bufferSize)
{
#if defined(__APPLE__)
    return aligned_alloc(dynamicBufferAlignment, bufferSize);
#elif defined(_WIN32)
    return _aligned_malloc(bufferSize, dynamicBufferAlignment);
#else
    return nullptr;
#endif
}

void AlignedFree(void* aligned)
{
#if defined(__APPLE__)
    free(aligned);
#elif defined(_WIN32)
    _aligned_free(aligned);
#endif
}