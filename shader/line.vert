#version 450
#include "common.glsl"

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 outColor;

layout (set = 0, binding = 0) uniform Camera {
    CameraData data;
} camera;

void main()
{
    gl_Position = camera.data.proj * camera.data.view * vec4(inPos, 1.0);
    outColor = inColor;
}
