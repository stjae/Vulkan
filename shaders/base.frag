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
layout (set = 1, binding = 1) uniform texture2DArray textures[];
layout (set = 1, binding = 2) uniform textureCube shadowCubeMaps[];

layout (location = 0) in vec4 inModel;
layout (location = 1) in vec4 inNormal;
layout (location = 2) in vec2 inTexcoord;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

layout (location = 5) flat in int instanceIndex;

layout (location = 0) out vec4 outColor;
layout (location = 1) out ivec2 outID;

void main() {

    MeshInstanceData meshInstance = mesh[pushConsts.meshIndex].data[instanceIndex];
    vec3 albedo = meshInstance.albedo;
    float metallic = meshInstance.metallic;
    float roughness = meshInstance.roughness;
    float ao = 1.0;

    outColor = vec4(meshInstance.albedo, 1.0);
    vec3 normalWorld = inNormal.xyz;

    if (pushConsts.materialID > -1) {
        vec4 texColor = texture(sampler2DArray(textures[pushConsts.materialID], texSampler), vec3(inTexcoord, 0));
        if (texColor.a < 1.0) {
            discard;
        }
        if (length(texColor.rgb) != 0.0) {
            albedo = texColor.rgb;
        }
        vec3 texNormal = texture(sampler2DArray(textures[pushConsts.materialID], texSampler), vec3(inTexcoord, 1)).rgb;
        if (length(texNormal.rgb) != 0.0) {
            texNormal = 2.0 * texNormal - 1.0;

            vec3 N = inNormal.xyz;
            vec3 T = normalize(inTangent - dot(inTangent, N) * N);
            vec3 B = inBitangent;

            mat3 TBN = mat3(T, B, N);
            normalWorld = TBN * normalize(texNormal);
        }
    }

    vec3 worldModel = inModel.xyz;
    vec3 N = normalWorld;
    vec3 V = normalize(camera.pos - worldModel);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < pushConsts.lightCount; i++) {

        vec4 lightPos = light.data[i].model * vec4(light.data[i].pos, 1.0);
        vec3 lightVec = (inModel - lightPos).xyz;
        float sampledDist = texture(samplerCube(shadowCubeMaps[i], texSampler), lightVec).r;
        float distToLight = length(lightVec);
        float offset = 0.005;
        bool castShadow = distToLight > sampledDist + offset;

        vec3 radiance = Lambert(normalWorld, inModel.xyz, light.data[i]) * light.data[i].color;

        vec3 L = normalize(lightPos.xyz - worldModel);
        vec3 H = normalize(V + L);

        // brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        float NdotL = max(dot(N, L), 0.0);
        if (!castShadow)
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    //    outColor.rgb = color;
    outColor.rgb = normalWorld.rgb;

    outID.r = meshInstance.meshID;
    outID.g = meshInstance.instanceID;
}