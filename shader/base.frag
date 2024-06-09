#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#include "common.glsl"
#include "pbr.glsl"
#define SHADOW_MAP_CASCADE_COUNT 4

layout (set = 0, binding = 0) uniform Camera {
    CameraData data;
} camera;

layout (set = 0, binding = 1) readonly buffer Light {
    LightData data[];
} light;

layout (set = 0, binding = 2) readonly buffer Mesh {
    MeshInstanceData data[];
} mesh[];

layout (set = 3, binding = 0) uniform Cascade {
    vec4 splits;
    mat4 viewProj[SHADOW_MAP_CASCADE_COUNT];
    vec3 lightDir;
    int debug;
    vec3 color;
    float intensity;
} cascade;

layout (push_constant) uniform PushConsts
{
    int meshIndex;
    int materialID;
    int lightCount;
    float iblExposure;
} pushConsts;

layout (set = 1, binding = 0) uniform sampler repeatSampler;
layout (set = 1, binding = 1) uniform texture2D albedoTex[];
layout (set = 1, binding = 2) uniform texture2D normalTex[];
layout (set = 1, binding = 3) uniform texture2D metallicTex[];
layout (set = 1, binding = 4) uniform texture2D roughnessTex[];
layout (set = 1, binding = 5) uniform textureCube shadowCubeMaps[];

layout (set = 2, binding = 0) uniform samplerCube irradianceCubemap;
layout (set = 2, binding = 1) uniform samplerCube prefilteredCubemap;
layout (set = 2, binding = 2) uniform sampler2D brdfLUT;
layout (set = 2, binding = 3) uniform sampler2DArray shadowMap;

layout (location = 0) in vec4 inWorldPos;
layout (location = 1) in vec4 inNormal;
layout (location = 2) in vec2 inTexcoord;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;
layout (location = 5) in vec4 inViewPos;

layout (location = 6) flat in int instanceIndex;

layout (location = 0) out vec4 outColor;
layout (location = 1) out ivec2 outID;

const float SHADOW_OFFSET = 0.005;

float textureProj(vec4 shadowCoord, vec2 offset, uint cascadeIndex)
{
    float shadow = 0.0;
    float bias = 0.005;

    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0) {
        float dist = texture(shadowMap, vec3(shadowCoord.st + offset, cascadeIndex)).r;
        if (shadowCoord.w > 0 && dist < shadowCoord.z - bias) {
            // cast no shadow
            shadow = 1.0;
        }
    }
    return shadow;
}

const mat4 biasMat = mat4(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.5, 0.5, 0.0, 1.0
);

void main() {

    MeshInstanceData meshInstance = mesh[pushConsts.meshIndex].data[instanceIndex];
    vec3 albedo = meshInstance.albedo;
    float metallic = meshInstance.metallic;
    float roughness = meshInstance.roughness;
    float ao = 1.0;

    outColor = vec4(meshInstance.albedo, 1.0);
    vec3 normalWorld = inNormal.xyz;

    if (pushConsts.materialID > -1 && meshInstance.useTexture > 0) {
        vec4 texColor = texture(sampler2D(albedoTex[pushConsts.materialID], repeatSampler), inTexcoord);
        if (texColor.a < 1.0) {
            discard;
        }
        texColor.rgb = pow(texColor.rgb, vec3(2.2));
        if (length(texColor.rgb) != 0.0) {
            albedo = texColor.rgb;
        }
        vec3 texNormal = texture(sampler2D(normalTex[pushConsts.materialID], repeatSampler), inTexcoord).rgb;
        if (length(texNormal.rgb) != 0.0) {
            texNormal = 2.0 * texNormal - 1.0;

            vec3 N = inNormal.xyz;
            vec3 T = normalize(inTangent - dot(inTangent, N) * N);
            vec3 B = inBitangent;

            mat3 TBN = mat3(T, B, N);
            normalWorld = TBN * normalize(texNormal);
        }

        vec4 texMetallic = texture(sampler2D(metallicTex[pushConsts.materialID], repeatSampler), inTexcoord);
        // check if it's dummy texture
        if (length(texMetallic.rgb) != 0.0) {
            metallic = texMetallic.b;
        }
        vec4 texRoughness = texture(sampler2D(roughnessTex[pushConsts.materialID], repeatSampler), inTexcoord);
        // check if it's dummy texture
        if (length(texRoughness.rgb) != 0.0) {
            roughness = texRoughness.g;
        }
    }

    vec3 worldPos = inWorldPos.xyz;
    vec3 N = normalWorld;
    vec3 V = normalize(camera.data.pos - worldPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    uint cascadeIndex = 0;
    for (uint i = 0; i < SHADOW_MAP_CASCADE_COUNT - 1; ++i) {
        if (inViewPos.z < cascade.splits[i]) {
            cascadeIndex = i + 1;
        }
    }
    float shadow = 1.0;
    vec4 shadowCoord = (biasMat * cascade.viewProj[cascadeIndex]) * vec4(worldPos, 1.0);
    shadow = textureProj(shadowCoord / shadowCoord.w, vec2(0.0), cascadeIndex);
    if (shadow < 1.0) {
        vec3 L = -cascade.lightDir;
        vec3 radiance = max(0.0, dot(L, N)) * cascade.color * cascade.intensity;

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
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    for (int i = 0; i < pushConsts.lightCount; i++) {
        vec4 lightPos = light.data[i].model * vec4(0.0, 0.0, 0.0, 1.0);
        vec3 lightVec = (inWorldPos - lightPos).xyz;
        float sampledDist = texture(samplerCube(shadowCubeMaps[i], repeatSampler), lightVec).r;
        float distToLight = length(lightVec);
        // TODO: Attenuation
        bool castShadow = distToLight > sampledDist + SHADOW_OFFSET;
        if (castShadow) {
            continue;
        }

        vec3 radiance = Lambert(normalWorld, inWorldPos.xyz, light.data[i]) * light.data[i].color;

        vec3 L = normalize(lightPos.xyz - worldPos);
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
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceCubemap, N).rgb * pushConsts.iblExposure;
    vec3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilteredCubemap, R, roughness * MAX_REFLECTION_LOD).rgb * pushConsts.iblExposure;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;

    vec3 color = ambient + Lo;
    outColor.rgb = color;

    outID.r = meshInstance.meshID;
    outID.g = meshInstance.instanceID;

    if (cascade.debug > 0) {
        switch (cascadeIndex) {
            case 0:
                outColor.rgb *= vec3(1.0f, 0.25f, 0.25f);
                break;
            case 1:
                outColor.rgb *= vec3(0.25f, 1.0f, 0.25f);
                break;
            case 2:
                outColor.rgb *= vec3(0.25f, 0.25f, 1.0f);
                break;
            case 3:
                outColor.rgb *= vec3(1.0f, 1.0f, 0.25f);
                break;
        }
    }
}