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

layout (set = 3, binding = 0) uniform ShadowMap {
    mat4 viewProj;
} shadowMap;

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

layout (location = 0) out vec4 outModel;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec2 outTexcoord;
layout (location = 3) out vec3 outTangent;
layout (location = 4) out vec3 outBitangent;
layout (location = 5) out vec4 outShadowMapFragPos;

layout (location = 6) out int outInstanceIndex;

void main() {

    outModel = mesh[pushConsts.meshIndex].data[gl_InstanceIndex].model * vec4(inPos, 1.0);
    gl_Position = camera.data.proj * camera.data.view * outModel;
    outNormal = normalize(mesh[pushConsts.meshIndex].data[gl_InstanceIndex].invTranspose * vec4(inNormal, 0.0));
    outTexcoord = inTexcoord;
    outTangent = inTangent;
    outBitangent = inBitangent;
    outShadowMapFragPos = shadowMap.viewProj * outModel;

    outInstanceIndex = gl_InstanceIndex;
}