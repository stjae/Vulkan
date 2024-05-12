#include "meshModel.h"

MeshModel::MeshModel(int meshID, const std::string& filePath) : Mesh(meshID)
{
    if (!filePath.empty()) {
        filepath_ = filePath;
        LoadModel(filePath);
    }
}

void MeshModel::LoadModel(const std::string& filepath)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filepath.c_str(),
                                             aiProcess_CalcTangentSpace |
                                                 aiProcess_JoinIdenticalVertices |
                                                 aiProcess_Triangulate |
                                                 aiProcess_FlipUVs |
                                                 aiProcess_GenNormals);

    if (scene == nullptr || scene->mRootNode == nullptr) {
        spdlog::error(importer.GetErrorString());
        return;
    }

    aiNode* node = scene->mRootNode;
    ProcessNode(node, scene);

    if (scene->HasMaterials()) {
        materials_.reserve(scene->mNumMaterials);
        for (unsigned int m = 0; m < scene->mNumMaterials; m++) {
            aiString path;
            materials_.emplace_back();
            scene->mMaterials[m]->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &path);
            materials_.back().albedo = path.C_Str();
            scene->mMaterials[m]->GetTexture(aiTextureType_NORMALS, 0, &path);
            materials_.back().normal = path.C_Str();
            scene->mMaterials[m]->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &path);
            materials_.back().metallic = path.C_Str();
            scene->mMaterials[m]->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &path);
            materials_.back().roughness = path.C_Str();
        }
    }

    // TODO:
    name_ = filepath.substr(filepath.find_last_of("/\\") + 1, filepath.rfind('.') - filepath.find_last_of("/\\") - 1);
}

void MeshModel::ProcessNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        node->mTransformation.Transpose();
        glm::mat4 modelMat(1.0f);
        modelMat[0] = { node->mTransformation.a1, node->mTransformation.a2, node->mTransformation.a3, node->mTransformation.a4 };
        modelMat[1] = { node->mTransformation.b1, node->mTransformation.b2, node->mTransformation.b3, node->mTransformation.b4 };
        modelMat[2] = { node->mTransformation.c1, node->mTransformation.c2, node->mTransformation.c3, node->mTransformation.c4 };
        modelMat[3] = { node->mTransformation.d1, node->mTransformation.d2, node->mTransformation.d3, node->mTransformation.d4 };

        ProcessLoadedMesh(mesh, modelMat);
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
    for (auto& bulletMesh : m_bulletMeshes) {
        m_bulletVertexArray.addIndexedMesh(bulletMesh);
    }
}

void MeshModel::ProcessLoadedMesh(aiMesh* mesh, glm::mat4& modelMat)
{
    vertexContainers_.emplace_back();
    indexContainers_.emplace_back();
    posContainers_.emplace_back();

    aiFace* faces = mesh->mFaces;
    for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
        unsigned int* indices = faces[f].mIndices;
        indexContainers_.back().push_back(indices[0]);
        indexContainers_.back().push_back(indices[1]);
        indexContainers_.back().push_back(indices[2]);
    }

    for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
        glm::vec4 pos(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z, 1.0f);
        pos = modelMat * pos;

        glm::vec3 normal(0.0f);
        if (mesh->HasNormals())
            normal = { mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z };
        glm::vec2 texcoord(0.0f);
        if (mesh->HasTextureCoords(0))
            texcoord = { mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y };
        glm::vec3 tangent(0.0f);
        glm::vec3 biTangent(0.0f);
        if (mesh->HasTangentsAndBitangents()) {
            tangent = { mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z };
            biTangent = { mesh->mBitangents[v].x, mesh->mBitangents[v].y, mesh->mBitangents[v].z };
        }
        vertexContainers_.back().emplace_back(glm::vec3(pos.x, pos.y, pos.z),
                                              normal,
                                              texcoord,
                                              tangent,
                                              biTangent);

        posContainers_.back().push_back(pos.x);
        posContainers_.back().push_back(pos.y);
        posContainers_.back().push_back(pos.z);
    }

    meshParts_.emplace_back(meshParts_.size(), mesh->mMaterialIndex);

    m_bulletMeshes.emplace_back();
    m_bulletMeshes.back().m_triangleIndexBase = (unsigned char*)indexContainers_.back().data();
    m_bulletMeshes.back().m_triangleIndexStride = 3 * sizeof(unsigned int);
    m_bulletMeshes.back().m_vertexBase = (unsigned char*)posContainers_.back().data();
    std::cout << posContainers_.back().size() << '\n';
    m_bulletMeshes.back().m_vertexStride = 3 * sizeof(float);
    m_bulletMeshes.back().m_numTriangles = mesh->mNumFaces;
    m_bulletMeshes.back().m_numVertices = mesh->mNumVertices;
}
