#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexcoord;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

layout (location = 0) out vec3 localPos;

layout(push_constant) uniform PushConsts
{
    mat4 view;
    mat4 proj;
} pushConsts;

void main()
{
    localPos = inPos;
    gl_Position =  pushConsts.proj * pushConsts.view * vec4(localPos, 1.0);
}
