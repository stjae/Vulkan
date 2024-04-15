#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#include "common.glsl"

layout (push_constant) uniform PushConsts
{
    int meshIndex;
    int materialID;
    int lightCount;
    int useIBL;
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

    vec3 worldModel = inModel.xyz;
    vec3 N = normalWorld;
    vec3 V = normalize(camera.pos - worldModel);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < pushConsts.lightCount; i++) {

        vec4 lightPos = light.data[i].model * vec4(light.data[i].pos, 1.0);
        vec3 lightVec = (inModel - lightPos).xyz;
        float sampledDist = texture(samplerCube(shadowCubeMaps[i], repeatSampler), lightVec).r;
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

    vec3 ambient;
    if (pushConsts.useIBL > 0) {

        vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - metallic;

        vec3 irradiance = texture(irradianceCubemap, N).rgb;
        vec3 diffuse = irradiance * albedo;

        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(prefilteredCubemap, R, roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

        ambient = (kD * diffuse + specular) * ao;

    } else {

        ambient = vec3(0.03) * albedo * ao;
    }

    vec3 color = ambient + Lo;
    outColor.rgb = color;

    outID.r = meshInstance.meshID;
    outID.g = meshInstance.instanceID;
}