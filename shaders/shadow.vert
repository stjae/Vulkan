#version 450
#include "common.glsl"

layout(push_constant) uniform PushConsts
{
    mat4 view;
    vec3 padding;
    int lightIndex;
} pushConsts;

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertNormal;
layout (location = 2) in vec3 vertColor;
layout (location = 3) in vec2 inTexcoord;

layout (location = 0) out vec4 outPos;
layout (location = 1) out vec4 outLightPos;

void main() {

    vec4 worldPos = mesh.data[gl_InstanceIndex].model * vec4(vertPos, 1.0);

    gl_Position = camera.proj * pushConsts.view * worldPos;

    outPos = worldPos;
    outLightPos = light.data[pushConsts.lightIndex].model * vec4(light.data[pushConsts.lightIndex].pos, 1.0);
}
