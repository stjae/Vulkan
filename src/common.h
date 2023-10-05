#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef NDEBUG
const bool debug = false;
#else
const bool debug = true;
#endif

// glfw, vulkan library
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

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

#include "API/device.h"
#include "API/instance.h"
#include "API/log.h"
#include "API/window.h"

static std::vector<char> ReadFile(const std::string& filename);

#endif