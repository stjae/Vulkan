#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#include "common.glsl"

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 outColor;

layout (set = 0, binding = 0) uniform Camera {
    CameraData data;
} camera;

layout (set = 0, binding = 1) readonly buffer Mesh {
    MeshInstanceData data[];
} mesh[];

layout (push_constant) uniform PushConsts
{
    int meshIndex;
} pushConsts;

void main()
{
    vec4 worldModel = mesh[pushConsts.meshIndex].data[gl_InstanceIndex].model * vec4(inPos, 1.0);
    gl_Position = camera.data.proj * camera.data.view * worldModel;
    outColor = inColor;
}
