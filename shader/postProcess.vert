#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexcoord;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

layout(location = 0) out vec2 outTexcoord;

void main() {
    gl_Position = vec4(inPos, 1.0f);
    outTexcoord = inTexcoord;
}