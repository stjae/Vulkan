#ifndef LIGHT_H
#define LIGHT_H

#include "../common.h"

struct LightData
{
    friend class Scene;

    glm::mat4 model;
    glm::vec3 pos;

    LightData() : model(glm::mat4(1.0f)), pos(glm::vec3(0.0f)), maxLights(0), color(1.0f), padding(0.0f) {}

private:
    int32_t maxLights;

public:
    glm::vec3 color;
    float padding;
};

#endif