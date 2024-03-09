#version 450
#include "common.glsl"

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertNormal;
layout (location = 2) in vec3 vertColor;
layout (location = 3) in vec2 inTexcoord;

layout (location = 0) out vec4 worldModel;
layout (location = 1) out vec4 worldNormal;
layout (location = 2) out vec3 outColor;
layout (location = 3) out vec2 outTexcoord;

layout (location = 4) out int outInstanceIndex;

void main() {

    worldModel = mesh.data[gl_InstanceIndex].model * vec4(vertPos, 1.0);
    gl_Position = camera.proj * camera.view * worldModel;
    worldNormal = normalize(mesh.data[gl_InstanceIndex].invTranspose * vec4(vertNormal, 0.0));
    outColor = vertColor;
    outTexcoord = inTexcoord;

    outInstanceIndex = gl_InstanceIndex;
}