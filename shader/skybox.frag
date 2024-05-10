#version 450

layout (push_constant) uniform PushConsts
{
    float exposure;
} pushConsts;

layout (location = 0) in vec3 localPos;
layout (location = 0) out vec4 fragColor;

layout (binding = 1) uniform samplerCube envMap;

void main()
{
    vec3 envColor = texture(envMap, localPos).rgb;
    envColor *= pushConsts.exposure;

    fragColor = vec4(envColor, 1.0);
}
