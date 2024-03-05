#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#include "common.glsl"

layout(set = 1, binding = 0) uniform sampler2D texSampler[];

layout(location = 0) in vec4 worldModel;
layout(location = 1) in vec4 worldNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexcoord;

layout(location = 4) flat in int useTexture;
layout(location = 5) flat in int meshID;
layout(location = 6) flat in int textureID;
layout(location = 7) flat in int instanceID;

layout(location = 0) out vec4 outColor;
layout(location = 1) out ivec2 outID;

void main() {

    vec4 dstColor = vec4(inColor, 1.0);

    if (useTexture != 0) {
        dstColor = texture(texSampler[textureID], inTexcoord);
    }

    dstColor.rgb += Lambert(worldNormal.xyz, worldModel.xyz) * dstColor.rgb;
    outColor = dstColor;
    outID.r = meshID;
    outID.g = instanceID;
}