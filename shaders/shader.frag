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
} ubo[];

layout(binding = 1) uniform LightUBO {
    vec3 pos;
} lightUBO;

layout (binding = 2) uniform sampler2D textureSampler[];

layout(location = 0) in vec4 modelWorld;
layout(location = 1) in vec3 normalWorld;
layout(location = 2) in vec2 fragTexcoord;

layout(location = 0) out vec4 outColor;

void main() {
    // vec3 toLight = lightUBO.pos - modelWorld.xyz;
    // vec3 toEye = ubo.eye - modelWorld.xyz;

    // vec3 halfway = normalize(toLight + toEye);

    // outColor = vec4(vec3(1.0) * max(dot(halfway, normalWorld), 0.0), 1.0);
    // outColor += vec4(fragTexcoord, 0.0, 1.0);
    outColor = texture(textureSampler[pushConstant.meshIndex], fragTexcoord) * 2.0;
}