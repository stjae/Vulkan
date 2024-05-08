struct CameraData
{
    mat4 view;
    mat4 proj;
    vec3 pos;
};

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
};

struct LightData
{
    mat4 model;
    vec3 pos;
    float padding0;
    vec3 color;
    float padding1;
};

float Lambert(vec3 worldNormal, vec3 worldModel, LightData lightData)
{
    vec4 lightPos = lightData.model * vec4(lightData.pos, 1.0);
    vec3 N = worldNormal;
    vec3 L = normalize(lightPos.xyz - worldModel);

    return max(0.0, dot(L, N));
}
