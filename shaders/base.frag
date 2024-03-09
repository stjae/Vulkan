#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#include "common.glsl"

layout (set = 1, binding = 0) uniform sampler2D texSampler[];

layout (location = 0) in vec4 worldModel;
layout (location = 1) in vec4 worldNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec2 inTexcoord;

layout (location = 4) flat in int instanceIndex;

layout (location = 0) out vec4 outColor;
layout (location = 1) out ivec2 outID;

void main() {

    MeshInstanceData meshInstance = mesh.data[instanceIndex];

    vec4 dstColor = vec4(inColor, 1.0);

    if (meshInstance.useTexture != 0) {
        dstColor = texture(texSampler[meshInstance.textureID], inTexcoord);
    }

    for (int i = 0; i < light.data[0].maxLights; i++) {
        dstColor.rgb += Lambert(worldNormal.xyz, worldModel.xyz, light.data[i]) * light.data[i].color * dstColor.rgb;
    }

    outColor = dstColor;
    outID.r = meshInstance.meshID;
    outID.g = meshInstance.instanceID;
}