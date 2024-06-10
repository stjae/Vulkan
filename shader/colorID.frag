#version 450

layout (location = 0) flat in int inMeshID;
layout (location = 1) flat in int inInstanceID;

layout (location = 0) out ivec2 outID;

void main() {

    outID.r = inMeshID;
    outID.g = inInstanceID;
}