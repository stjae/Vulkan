#version 450

layout(binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
} uboData;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

layout (push_constant) uniform constants {
    mat4 model;
} ObjectData;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = uboData.proj * uboData.view * uboData.model * vec4(vertexPosition, 1.0);
    fragColor = vertexColor;
}