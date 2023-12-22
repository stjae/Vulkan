#ifndef COMMON_H
#define COMMON_H

#ifdef NDEBUG
const bool debug = false;
#else
const bool debug = true;
#endif

// glfw, vulkan library
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#if defined(__APPLE__)
#include <MoltenVK/vk_mvk_moltenvk.h>
#endif

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_vulkan.h"
#if defined(_WIN32)
#include "../imgui/imgui_impl_win32.h"
#endif

// std library
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>

// external library
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/color.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "config.h"
#include "API/handle.h"

std::string& GetFrameRate();
std::vector<char> ReadFile(const std::string& filename);
void* AlignedAlloc(size_t dynamicBufferAlignment, size_t bufferSize);
void AlignedFree(void* aligned);

// spdlog::info
template <typename... T>
void Log(bool debug, fmt::terminal_color color, T... args)
{
    if (!debug) {
        return;
    }
    spdlog::info(fmt::format(fmt::fg(color), args...));
}

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texcoord;
    int textureID;
};

struct BufferInput
{
    size_t size;
    vk::BufferUsageFlags usage;
    vk::MemoryPropertyFlags properties;
};

#endif