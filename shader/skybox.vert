#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexcoord;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

layout (location = 0) out vec3 localPos;

layout (set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 proj;
    vec3 pos;
} camera;

void main()
{
    localPos = inPos;

    mat4 rotView = mat4(mat3(camera.view));// remove translation from the view matrix
    vec4 clipPos = camera.proj * rotView * vec4(localPos, 1.0);

    gl_Position = clipPos.xyww;
}