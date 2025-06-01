struct CameraData
{
    mat4 view;
    mat4 proj;
    vec3 pos;
};

struct MeshInstanceData
{
    mat4 model;
    int meshID;
    int instanceID;
    int useAlbedoTexture;
    int useNormalTexture;
    int useMetallicTexture;
    int useRoughnessTexture;
    float padding0[2];
    vec3 albedo;
    float metallic;
    float roughness;
    float padding1[3];
};

struct LightData
{
    vec3 pos;
    float intensity;
    vec3 color;
    float range;
};