#ifndef _MESH_H_
#define _MESH_H_

#include "meshData.h"

class Mesh : public MeshData
{
public:
    void CreateSquare();
    void CreateCube();
    void LoadModel(const char* modelPath, const char* texturePath);
};

#endif