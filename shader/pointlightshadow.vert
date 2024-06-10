#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#include "common.glsl"

layout (set = 0, binding = 0) uniform Proj {
    mat4 mat;
} proj;

layout (set = 0, binding = 1) readonly buffer Light {
    LightData data[];
} light;

layout (set = 0, binding = 2) readonly buffer Mesh {
    MeshInstanceData data[];
} mesh[];

layout (push_constant) uniform PushConsts
{
    mat4 view;
    vec2 padding;
    int meshIndex;
    int lightIndex;
} pushConsts;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec2 inTexcoord;
layout (location = 4) in vec3 inTangent;

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec3 outLightPos;

void main() {

    vec4 worldPos = mesh[pushConsts.meshIndex].data[gl_InstanceIndex].model * vec4(inPos, 1.0);

    gl_Position = proj.mat * pushConsts.view * worldPos;

    outPos = worldPos.xyz;
    outLightPos = light.data[pushConsts.lightIndex].pos;
}
