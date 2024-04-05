#include "brdf.glsl"

struct MeshInstanceData
{
    mat4 model;
    mat4 invTranspose;
    int meshID;
    int textureID;
    int instanceID;
    int useTexture;
    vec3 albedo;
    float metallic;
    float roughness;
    float padding[3];
};

struct LightData
{
    mat4 model;
    vec3 pos;
    float padding0;
    vec3 color;
    float padding1;
};

layout (set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 proj;
    vec3 pos;
} camera;

layout (set = 0, binding = 1) readonly buffer Light {
    LightData data[];
} light;

layout (set = 0, binding = 2) readonly buffer Mesh {
    MeshInstanceData data[];
} mesh[];

float Lambert(vec3 worldNormal, vec3 worldModel, LightData lightData)
{
    vec4 lightPos = lightData.model * vec4(lightData.pos, 1.0);
    vec3 N = worldNormal;
    vec3 L = normalize(lightPos.xyz - worldModel);

    float dist = length(worldModel - lightPos.xyz);
    float attenuation = 1.0 / (1.0 + (0.25 * dist * dist));
    return max(0.0, dot(L, N)) * attenuation;
}
