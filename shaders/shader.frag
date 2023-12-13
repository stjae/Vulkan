#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(push_constant) uniform PushConstant {
    uint meshIndex;
} pushConstant;

layout(set = 0, binding = 0) uniform CameraMatrix {
    mat4 view;
    mat4 proj;
} cameraMatrix;

layout(set = 1, binding = 0) uniform ModelMatrix {
    mat4 model;
} modelMatrix;

layout(set = 2, binding = 0) uniform sampler2D texSampler[];

layout(location = 0) in vec4 modelWorld;
layout(location = 1) in vec3 normalWorld;
layout(location = 2) in vec2 fragTexcoord;

layout(location = 0) out vec4 outColor;

void main() {

    outColor = texture(texSampler[nonuniformEXT(pushConstant.meshIndex)], fragTexcoord);
    // outColor = vec4(modelWorld.x, modelWorld.y, modelWorld.z, 1.0);
}