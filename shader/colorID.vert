#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#include "common.glsl"

layout (set = 0, binding = 0) uniform Camera {
    CameraData data;
} camera;

layout (set = 0, binding = 1) readonly buffer Mesh {
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

layout (location = 0) out int outMeshID;
layout (location = 1) out int outInstanceID;

void main() {

    vec4 worldPos = mesh[pushConsts.meshIndex].data[gl_InstanceIndex].model * vec4(inPos, 1.0);
    gl_Position = camera.data.proj * camera.data.view * worldPos;

    MeshInstanceData meshInstance = mesh[pushConsts.meshIndex].data[gl_InstanceIndex];
    outMeshID = meshInstance.meshID;
    outInstanceID = meshInstance.instanceID;
}
