#version 450
// #extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) uniform CameraMatrix {
    mat4 view;
    mat4 proj;
} cameraMat;

layout(set = 1, binding = 0) uniform ModelMatrix {
    mat4 model;
} modelMat;

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in int inTexID;
layout(location = 4) in int inMeshID;

layout(location = 0) out vec4 worldModel;
layout(location = 1) out vec3 worldNormal;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) out flat int outTexID;
layout(location = 4) out flat int outMeshID;

void main() {
    worldModel = modelMat.model * vec4(vertPos, 1.0);
    gl_Position = cameraMat.proj * cameraMat.view * worldModel;

    worldNormal = vertNormal;
    outTexCoord = inTexCoord;
    outTexID = inTexID;
    outMeshID = inMeshID;
}