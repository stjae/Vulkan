#version 450

layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 proj;
} camera;

layout(set = 1, binding = 0) uniform Mesh {
    mat4 model;
    int id;
    int textureId;
    bool useTexture;
} mesh;

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec3 vertColor;
layout(location = 3) in vec2 inTexcoord;

void main() {
    vec4 worldModel = mesh.model * vec4(vertPos, 1.0);
    gl_Position = camera.proj * camera.view * worldModel;
}
