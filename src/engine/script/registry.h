#ifndef REGISTRY_H
#define REGISTRY_H

#include <mono/jit/jit.h>
#include <iostream>
#include "spdlog/spdlog.h"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"

class Registry
{
public:
    static void RegisterFunctions();
};

#endif
