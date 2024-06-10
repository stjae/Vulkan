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
    int textureID;
    int instanceID;
    int useTexture;
    vec3 albedo;
    float metallic;
    float roughness;
};

struct LightData
{
    vec3 pos;
    float intensity;
    vec3 color;
    float range;
};
