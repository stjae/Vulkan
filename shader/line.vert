#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#include "common.glsl"

struct PhysicsDebugData
{
    mat4 model;
    vec3 scale;
    int havePhysicsInfo;
};

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 outColor;
layout (location = 1) out flat int outHavePhysicsInfo;

layout (set = 0, binding = 0) uniform Camera {
    CameraData data;
} camera;

layout (set = 0, binding = 1) uniform Line {
    PhysicsDebugData data;
} line;

layout (push_constant) uniform PushConsts
{
    int meshIndex;
} pushConsts;

void main()
{
    vec3 scaleVec = line.data.scale;
    mat4 scaleMat = mat4(1.0);
    scaleMat[0].x = scaleVec.x;
    scaleMat[1].y = scaleVec.y;
    scaleMat[2].z = scaleVec.z;
    vec4 worldModel = line.data.model * scaleMat * vec4(inPos, 1.0);
    gl_Position = camera.data.proj * camera.data.view * worldModel;
    outColor = inColor;
    outHavePhysicsInfo = line.data.havePhysicsInfo;
}
