#include "common.h"

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
        Log(debug, fmt::terminal_color::white, "{0} loaded with code length of {1}", filename, buffer.size());
    }

    return buffer;
}