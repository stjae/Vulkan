#version 450

layout (location = 0) out float outColor;

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec4 inLightPos;

void main() {

    vec3 lightVec = inPos.xyz - inLightPos.xyz;
    outColor = length(lightVec);
}