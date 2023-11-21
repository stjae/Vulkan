#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(push_constant) uniform constants
{
    int meshIndex;
} pushConstant;

layout(binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 eye;
} ubo[2];

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexcoord;

layout(location = 0) out vec4 modelWorld;
layout(location = 1) out vec3 normalWorld;
layout(location = 2) out vec2 fragTexcoord;

void main() {
    modelWorld = ubo[pushConstant.meshIndex].model * vec4(vertexPosition, 1.0);
    normalWorld = vertexNormal;
    gl_Position = ubo[pushConstant.meshIndex].proj * ubo[pushConstant.meshIndex].view * ubo[pushConstant.meshIndex].model * vec4(vertexPosition, 1.0);
    fragTexcoord = vertexTexcoord;
}