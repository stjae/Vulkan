#version 450
#include "common.glsl"

layout (set = 0, binding = 0) uniform PrevCamera {
    CameraData data;
} prevCamera;
layout (set = 0, binding = 1) uniform Camera {
    CameraData data;
} camera;
layout (set = 0, binding = 2) uniform sampler2D renderImage;
layout (set = 0, binding = 3) uniform sampler2D depthMap;

layout (location = 0) in vec2 inTexCoord;
layout (location = 0) out vec4 outColor;

void main() {
    float zOverW = texture(depthMap, inTexCoord).r;
    vec4 H = vec4(inTexCoord.x * 2 - 1, (1 - inTexCoord.y) * 2 - 1, zOverW, 1);
    mat4 invViewProj = inverse(camera.data.proj * camera.data.view);
    vec4 D = invViewProj * H;
    vec4 worldPos = D / D.w;

    vec4 currentPos = H;
    vec4 prevPos = prevCamera.data.proj * prevCamera.data.view * worldPos;
    prevPos /= prevPos.w;
    vec2 velocity = (currentPos - prevPos).xy / 20.0;

    vec4 color = texture(renderImage, inTexCoord);
    vec2 texCoord = inTexCoord;
    texCoord += velocity;

    int numSamples = 10;
    for (int i = 1; i < numSamples; i++) {
        texCoord += velocity;
        vec4 currentColor = texture(renderImage, texCoord);
        color += currentColor;
    }

    vec3 avgColor = color.rgb / numSamples;
    vec4 finalColor = vec4(avgColor, 1.0);
    outColor = finalColor;
}