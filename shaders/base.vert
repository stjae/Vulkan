#version 450
#include "common.glsl"

struct MeshInstanceData
{
    mat4 model;
    mat4 invTranspose;
    int meshID;
    int textureID;
    int useTexture;
    int instanceID;
};

layout(set = 0, binding = 2) buffer MeshInstance {
    MeshInstanceData data[];
} meshInstance;

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec3 vertColor;
layout(location = 3) in vec2 inTexcoord;

layout(location = 0) out vec4 worldModel;
layout(location = 1) out vec4 worldNormal;
layout(location = 2) out vec3 outColor;
layout(location = 3) out vec2 outTexcoord;

layout(location = 4) out int useTexture;
layout(location = 5) out int meshID;
layout(location = 6) out int textureID;
layout(location = 7) out int instanceID;

void main() {
    worldModel = meshInstance.data[gl_InstanceIndex].model * vec4(vertPos, 1.0);
    gl_Position = camera.proj * camera.view * worldModel;
    worldNormal = normalize(meshInstance.data[gl_InstanceIndex].invTranspose * vec4(vertNormal, 0.0));
    outColor = vertColor;
    outTexcoord = inTexcoord;

    useTexture = meshInstance.data[gl_InstanceIndex].useTexture;
    meshID = meshInstance.data[gl_InstanceIndex].meshID;
    textureID = meshInstance.data[gl_InstanceIndex].textureID;
    instanceID = meshInstance.data[gl_InstanceIndex].instanceID;
}