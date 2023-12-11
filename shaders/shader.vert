#version 450
// #extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 0) uniform CameraMatrix {
    mat4 view;
    mat4 proj;
} cameraMatrix;

layout(binding = 1) uniform ModelMatrix {
    mat4 model;
} modelMatrix;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexcoord;

layout(location = 0) out vec4 modelWorld;
layout(location = 1) out vec3 normalWorld;
layout(location = 2) out vec2 fragTexcoord;

void main() {
    modelWorld = modelMatrix.model * vec4(vertexPosition, 1.0);
    normalWorld = vertexNormal;
    gl_Position = cameraMatrix.proj * cameraMatrix.view * modelWorld;
    fragTexcoord = vertexTexcoord;
}