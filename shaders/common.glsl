struct MeshInstanceData
{
    mat4 model;
    mat4 invTranspose;
    int meshID;
    int textureID;
    int useTexture;
    int instanceID;
};

struct LightData
{
    mat4 model;
    vec3 pos;
    int maxLights;
    vec3 color;
    float padding;
};

layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 proj;
    vec3 pos;
} camera;

layout(set = 0, binding = 1) readonly buffer Light {
    LightData data[];
} light;

layout(set = 0, binding = 2) readonly buffer Mesh {
    MeshInstanceData data[];
} mesh;

float Lambert(vec3 worldNormal, vec3 worldModel, LightData lightData)
{
    vec4 lightPos = lightData.model * vec4(lightData.pos, 1.0);
    vec3 L = normalize(lightPos.xyz - worldModel);
    vec3 N = worldNormal;

    return max(0.0, dot(L, N));
}
