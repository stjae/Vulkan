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
layout (set = 1, binding = 3) uniform sampler2D normalSampler[];

layout (location = 0) in vec4 inModel;
layout (location = 1) in vec4 inNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec2 inTexcoord;
layout (location = 4) in vec3 inTangent;

layout (location = 5) flat in int instanceIndex;

layout (location = 0) out vec4 outColor;
layout (location = 1) out ivec2 outID;

void main() {

    MeshInstanceData meshInstance = mesh[pushConsts.meshIndex].data[instanceIndex];

    outColor = vec4(inColor, 1.0);
    vec3 normalWorld = inNormal.xyz;

    if (meshInstance.useTexture && meshInstance.textureID > 0) {
        outColor = texture(texSampler[meshInstance.textureID], inTexcoord);
    }
    if (pushConsts.materialID > 0) {
        vec4 texColor = texture(diffSampler[pushConsts.materialID], inTexcoord);
        if (texColor.a < 1.0f) {
            discard;
        }
        if (length(texColor.rgb) != 0.0) {
            outColor = texColor;
        }
        vec3 texNormal = texture(normalSampler[pushConsts.materialID], inTexcoord).rgb;
        if (length(texNormal.rgb) != 0.0) {
            texNormal = 2.0 * texNormal - 1.0;

            vec3 N = inNormal.xyz;
            vec3 T = normalize(inTangent - dot(inTangent, N) * N);
            vec3 B = cross(N, T);

            mat3 TBN = mat3(T, B, N);
            normalWorld = TBN * normalize(texNormal);
        }
    }

    for (int i = 0; i < light.data[0].maxLights; i++) {

        vec4 lightPos = light.data[i].model * vec4(light.data[i].pos, 1.0);
        vec3 lightVec = (inModel - lightPos).xyz;
        float sampledDist = texture(shadowCubeMap[i], lightVec).r;
        float dist = length(lightVec);
        float offset = 0.15;

        outColor.rgb += (dist <= sampledDist + offset) ? Lambert(normalWorld, inModel.xyz, light.data[i]) * light.data[i].color * outColor.rgb : vec3(0.0);
    }

    outID.r = meshInstance.meshID;
    outID.g = meshInstance.instanceID;
}