#version 450
// #extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) uniform CameraMatrix {
    mat4 view;
    mat4 proj;
} cameraMatrix;

layout(set = 1, binding = 0) uniform ModelMatrix {
    mat4 model;
} modelMatrix;

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec2 vertTexcoord;
layout(location = 3) in int vertTextureID;

layout(location = 0) out vec4 modelWorld;
layout(location = 1) out vec3 normalWorld;
layout(location = 2) out vec2 fragTexcoord;
layout(location = 3) out flat int fragTextureID;

void main() {
    modelWorld = modelMatrix.model * vec4(vertPosition, 1.0);
    normalWorld = vertNormal;
    fragTexcoord = vertTexcoord;
    fragTextureID = vertTextureID;
    gl_Position = cameraMatrix.proj * cameraMatrix.view * modelWorld;
}