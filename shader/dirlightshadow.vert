#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#include "common.glsl"

layout (set = 0, binding = 0) uniform ShadowMapSpace {
    mat4 viewProj;
} shadowMapSpace;

layout (set = 0, binding = 1) readonly buffer Mesh {
    MeshInstanceData data[];
} mesh[];

layout (push_constant) uniform PushConsts
{
    mat4 viewProj;
    int meshIndex;
} pushConsts;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec2 inTexcoord;
layout (location = 4) in vec3 inTangent;

void main() {

    vec4 worldPos = mesh[pushConsts.meshIndex].data[gl_InstanceIndex].model * vec4(inPos, 1.0);
    gl_Position = pushConsts.viewProj * worldPos;

}
