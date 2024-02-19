#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#include "common.glsl"

layout(set = 2, binding = 0) uniform sampler2D texSampler[];

layout(location = 0) in vec4 worldModel;
layout(location = 1) in vec3 worldNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out int outMeshID;

void main() {

    vec4 dstColor;
    if (mesh.useTexture) {
        dstColor = texture(texSampler[mesh.textureId], inTexcoord);
    }
    else {
        dstColor = vec4(inColor, 1.0);
    }

    dstColor.rgb += Lambert(worldNormal, worldModel.rgb) * dstColor.rgb;
    outColor = dstColor;
    outMeshID = mesh.id;
}