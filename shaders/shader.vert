#version 450
#include "common.glsl"

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec3 vertColor;
layout(location = 3) in vec2 inTexcoord;

layout(location = 0) out vec4 worldModel;
layout(location = 1) out vec3 worldNormal;
layout(location = 2) out vec3 outColor;
layout(location = 3) out vec2 outTexcoord;

void main() {
    worldModel = mesh.model * vec4(vertPos, 1.0);
    gl_Position = camera.proj * camera.view * worldModel;
    worldNormal = vertNormal;
    outColor = vertColor;
    outTexcoord = inTexcoord;
}