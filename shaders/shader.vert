#version 450

layout(binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 eye;
} ubo;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexcoord;

layout(location = 0) out vec4 modelWorld;
layout(location = 1) out vec3 normalWorld;
layout(location = 2) out vec2 fragTexcoord;

void main() {
    modelWorld = ubo.model * vec4(vertexPosition, 1.0);
    normalWorld = vertexNormal;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(vertexPosition, 1.0);
    fragTexcoord = vertexTexcoord;
}