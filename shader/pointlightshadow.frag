#version 450

layout (location = 0) out float outColor;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inLightPos;

void main() {

    vec3 lightVec = inPos - inLightPos;
    outColor = length(lightVec);
}