#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#include "common.glsl"

layout(push_constant) uniform PushConsts
{
    int meshIndex;
    int materialID;
} pushConsts;

layout (set = 1, binding = 0) uniform sampler2D texSampler[];
layout (set = 1, binding = 1) uniform samplerCube shadowCubeMap[];
layout (set = 1, binding = 2) uniform sampler2D diffSampler[];

layout (location = 0) in vec4 worldModel;
layout (location = 1) in vec4 worldNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec2 inTexcoord;

layout (location = 4) flat in int instanceIndex;

layout (location = 0) out vec4 outColor;
layout (location = 1) out ivec2 outID;

void main() {

    MeshInstanceData meshInstance = mesh[pushConsts.meshIndex].data[instanceIndex];

    vec4 dstColor = vec4(inColor, 1.0);

    if (meshInstance.useTexture && meshInstance.textureID > 0) {
        dstColor = texture(texSampler[meshInstance.textureID], inTexcoord);
    }
    if (pushConsts.materialID > 0) {
        dstColor = texture(diffSampler[pushConsts.materialID], inTexcoord);
    }
    if (dstColor.a < 1.0f) {
        discard;
    }

    for (int i = 0; i < light.data[0].maxLights; i++) {

        vec4 lightPos = light.data[i].model * vec4(light.data[i].pos, 1.0);
        vec3 lightVec = (worldModel - lightPos).xyz;
        float sampledDist = texture(shadowCubeMap[i], lightVec).r;
        float dist = length(lightVec);
        float offset = 0.15;

        dstColor.rgb += (dist <= sampledDist + offset) ? Lambert(worldNormal.xyz, worldModel.xyz, light.data[i]) * light.data[i].color * dstColor.rgb : vec3(0.0);
    }

    outColor = dstColor;
    outID.r = meshInstance.meshID;
    outID.g = meshInstance.instanceID;
}