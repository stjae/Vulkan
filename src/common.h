#ifndef _COMMON_H_
#define _COMMON_H_

// glfw, vulkan library
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

// std library
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>

// external library
#include <spdlog/spdlog.h>

#include "API/window.h"

static std::vector<char> ReadFile(const std::string& filename);

#endif