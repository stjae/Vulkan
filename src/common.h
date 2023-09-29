#ifndef _COMMON_H_
#define _COMMON_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <cstdlib>
#include <set>
#include <fstream>
#include <iostream>

#include <spdlog/spdlog.h>

static std::vector<char> ReadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        SPDLOG_ERROR("failed to open file: {}", filename);
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    
    if(buffer.size() > 0) {
        SPDLOG_INFO("{0} loaded with code length of {1}", filename, buffer.size());
    }

    return buffer;
}

#endif