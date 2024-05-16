#ifndef MESHMODEL_H
#define MESHMODEL_H

#include "mesh.h"

class MeshModel : public Mesh
{
    std::string m_filepath;

    void LoadModel(const std::string& filepath);
    void ProcessNode(aiNode* node, const aiScene* scene);
    void ProcessLoadedMesh(aiMesh* mesh, glm::mat4& modelMat);

public:
    MeshModel(int meshID, const std::string& filePath);
    explicit MeshModel(int meshID) : Mesh(meshID) {}
};

#endif
