#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) uniform CameraMatrix {
    mat4 view;
    mat4 proj;
} cameraMat;

layout(set = 1, binding = 0) uniform ModelMatrix {
    mat4 model;
    int id;
    int texId;
} modelMat;

layout(set = 2, binding = 0) uniform sampler2D texSampler[];

layout(location = 0) in vec4 worldModel;
layout(location = 1) in vec3 worldNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out int outMeshID;

void main() {

    outColor = texture(texSampler[modelMat.texId], inTexCoord);
    outMeshID = modelMat.id;
}