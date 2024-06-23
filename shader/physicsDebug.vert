#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#include "common.glsl"

struct PhysicsDebugData
{
    mat4 model;
    vec3 scale;
};

layout (set = 0, binding = 0) uniform Camera {
    CameraData data;
} camera;

layout (set = 0, binding = 1) uniform Physics {
    PhysicsDebugData data;
} physics;

layout (push_constant) uniform PushConsts
{
    int meshIndex;
} pushConsts;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 0) out vec3 outColor;

void main()
{
    mat4 scaleMat = mat4(1.0);
    scaleMat[0].x = physics.data.scale.x;
    scaleMat[1].y = physics.data.scale.y;
    scaleMat[2].z = physics.data.scale.z;
    vec4 worldModel = physics.data.model * scaleMat * vec4(inPos, 1.0);
    gl_Position = camera.data.proj * camera.data.view * worldModel;
    outColor = inColor;
}
