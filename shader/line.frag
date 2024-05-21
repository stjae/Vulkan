#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) flat in int inHavePhysicsInfo;

layout (location = 0) out vec4 outColor;

void main()
{
    //    if (inHavePhysicsInfo < 1){
    //        discard;
    //    }
    outColor = vec4(inColor, 1.0);
}
