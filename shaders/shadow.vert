#version 450

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
} mesh;

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertNormal;
layout (location = 2) in vec3 vertColor;
layout (location = 3) in vec2 inTexcoord;

layout (location = 0) out vec4 worldModel;

void main() {

    worldModel = mesh.data[gl_InstanceIndex].model * vec4(vertPos, 1.0);
    gl_Position = camera.proj * camera.view * worldModel;
}
