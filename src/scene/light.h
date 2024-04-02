#ifndef LIGHT_H
#define LIGHT_H

#include "../common.h"

struct LightData
{
    glm::mat4 model;
    glm::vec3 pos;
    float padding0;
    glm::vec3 color;
    float padding1;

    LightData() : model(glm::mat4(1.0f)), pos(glm::vec3(0.0f)), color(1.0f), padding0(0.0f), padding1(0.0f) {}
    explicit LightData(glm::vec3&& pos) : model(glm::mat4(1.0f)), pos(pos), color(1.0f), padding0(0.0f), padding1(0.0f) {}
};

#endif