#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#include "common.glsl"

layout (set = 0, binding = 0) uniform Camera {
    CameraData data;
} camera;

layout (set = 0, binding = 1) readonly buffer Light {
    LightData data[];
};

layout (set = 0, binding = 2) readonly buffer Mesh {
    MeshInstanceData data[];
} mesh[];

layout (push_constant) uniform PushConsts
{
    int meshIndex;
    int materialID;
} pushConsts;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexcoord;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

layout (location = 0) out vec4 outWorldPos;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec2 outTexcoord;
layout (location = 3) out vec3 outTangent;
layout (location = 4) out vec3 outBitangent;
layout (location = 5) out vec4 outViewPos;

layout (location = 6) out int outInstanceIndex;

void main() {

    mat4 modelMat = mesh[pushConsts.meshIndex].data[gl_InstanceIndex].model;
    outWorldPos = modelMat * vec4(inPos, 1.0);
    gl_Position = camera.data.proj * camera.data.view * outWorldPos;

    mat4 invTranspose = modelMat;
    invTranspose[3] = vec4(0.0, 0.0, 0.0, 1.0);
    invTranspose = transpose(inverse(invTranspose));

    outNormal = normalize(invTranspose * vec4(inNormal, 0.0));
    outTexcoord = inTexcoord;
    outTangent = inTangent;
    outBitangent = inBitangent;
    outViewPos = camera.data.view * outWorldPos;

    outInstanceIndex = gl_InstanceIndex;
}