#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#include "common.glsl"

layout (push_constant) uniform PushConsts
{
    int meshIndex;
    int materialID;
    int lightCount;
} pushConsts;

layout (set = 1, binding = 0) uniform sampler texSampler;
layout (set = 1, binding = 1) uniform texture2D textures[];
layout (set = 1, binding = 2) uniform textureCube shadowCubeMaps[];
layout (set = 1, binding = 3) uniform texture2D diffs[];
layout (set = 1, binding = 4) uniform texture2D normals[];

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

    if (meshInstance.useTexture > 0 && meshInstance.textureID > 0) {
        outColor = texture(sampler2D(textures[meshInstance.textureID], texSampler), inTexcoord);
    }
    if (pushConsts.materialID > 0) {
        vec4 texColor = texture(sampler2D(diffs[pushConsts.materialID], texSampler), inTexcoord);
        if (texColor.a < 1.0) {
            discard;
        }
        if (length(texColor.rgb) != 0.0) {
            outColor = texColor;
        }
        vec3 texNormal = texture(sampler2D(normals[pushConsts.materialID], texSampler), inTexcoord).rgb;
        if (length(texNormal.rgb) != 0.0) {
            texNormal = 2.0 * texNormal - 1.0;

            vec3 N = inNormal.xyz;
            vec3 T = normalize(inTangent - dot(inTangent, N) * N);
            vec3 B = cross(N, T);

            mat3 TBN = mat3(T, B, N);
            normalWorld = TBN * normalize(texNormal);
        }
    }

    for (int i = 0; i < pushConsts.lightCount; i++) {

        vec4 lightPos = light.data[i].model * vec4(light.data[i].pos, 1.0);
        vec3 lightVec = (inModel - lightPos).xyz;
        float sampledDist = texture(samplerCube(shadowCubeMaps[i], texSampler), lightVec).r;
        float dist = length(lightVec);
        float offset = 0.005;

        outColor.rgb += (dist <= sampledDist + offset) ? Lambert(normalWorld, inModel.xyz, light.data[i]) * light.data[i].color * outColor.rgb : vec3(0.0);
    }

    outID.r = meshInstance.meshID;
    outID.g = meshInstance.instanceID;
}