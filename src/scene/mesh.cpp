#include "mesh.h"

Mesh::Mesh(MESHTYPE meshType) : meshID_(meshType), instanceID_(0)
{
    switch (meshType) {
    case MESHTYPE::SQUARE:
        CreateSquare();
        break;
    case MESHTYPE::CUBE:
        CreateCube();
        break;
    case MESHTYPE::SPHERE:
        CreateSphere();
        break;
    }
}

Mesh::Mesh(uint32_t meshID, const std::string& filePath) : instanceID_(0)
{
    if (!filePath.empty())
        LoadModel(filePath);
    this->meshID_ = meshID;
}

void Mesh::CreateSquare(float scale, glm::vec3 color, const char* texturePath)
{
    const int SQUARE_VERTEX_COUNT = 4;

    std::vector<glm::vec3> position = { { -1.0f, 0.0f, -1.0f },
                                        { -1.0f, 0.0f, 1.0f },
                                        { 1.0f, 0.0f, 1.0f },
                                        { 1.0f, 0.0f, -1.0f } };

    std::vector<glm::vec3> normal = { { 0.0f, 1.0f, 0.0f },
                                      { 0.0f, 1.0f, 0.0f },
                                      { 0.0f, 1.0f, 0.0f },
                                      { 0.0f, 1.0f, 0.0f } };

    std::vector<glm::vec2> texcoord = { { 0.0f, 0.0f },
                                        { 0.0f, 1.0f },
                                        { 1.0f, 1.0f },
                                        { 1.0f, 0.0f } };

    vertexContainers_.reserve(1);
    vertexContainers_.emplace_back();

    for (int i = 0; i < SQUARE_VERTEX_COUNT; i++) {

        position[i] *= scale;
        vertexContainers_.back().emplace_back(position[i], normal[i], color, texcoord[i], glm::vec3(0.0f));
    }

    indexContainers_.reserve(1);
    indexContainers_.emplace_back();
    indexContainers_.back() = { 0, 1, 2, 2, 3, 0 };

    name_ = "square";

    meshParts_.reserve(1);
    meshParts_.emplace_back(0, -1);
}

void Mesh::CreateCube(float scale, glm::vec3 color, const char* texturePath)
{
    const int CUBE_VERTEX_COUNT = 24;

    std::vector<glm::vec3> position = { { -1.0f, 1.0f, 1.0f },
                                        { -1.0f, -1.0f, 1.0f }, // front
                                        { 1.0f, -1.0f, 1.0f },
                                        { 1.0f, 1.0f, 1.0f },

                                        { -1.0f, 1.0f, -1.0f },
                                        { -1.0f, -1.0f, -1.0f }, // back
                                        { 1.0f, -1.0f, -1.0f },
                                        { 1.0f, 1.0f, -1.0f },

                                        { -1.0f, 1.0f, -1.0f },
                                        { -1.0f, 1.0f, 1.0f }, // top
                                        { 1.0f, 1.0f, 1.0f },
                                        { 1.0f, 1.0f, -1.0f },

                                        { -1.0f, -1.0f, 1.0f },
                                        { -1.0f, -1.0f, -1.0f }, // bottom
                                        { 1.0f, -1.0f, -1.0f },
                                        { 1.0f, -1.0f, 1.0f },

                                        { 1.0f, 1.0f, 1.0f },
                                        { 1.0f, -1.0f, 1.0f }, // right
                                        { 1.0f, -1.0f, -1.0f },
                                        { 1.0f, 1.0f, -1.0f },

                                        { -1.0f, 1.0f, -1.0f },
                                        { -1.0f, -1.0f, -1.0f }, // left
                                        { -1.0f, -1.0f, 1.0f },
                                        { -1.0f, 1.0f, 1.0f } };

    std::vector<glm::vec3> normal = { { 0.0f, 0.0f, 1.0f },
                                      { 0.0f, 0.0f, 1.0f },
                                      { 0.0f, 0.0f, 1.0f },
                                      { 0.0f, 0.0f, 1.0f }, // front

                                      { 0.0f, 0.0f, -1.0f },
                                      { 0.0f, 0.0f, -1.0f },
                                      { 0.0f, 0.0f, -1.0f },
                                      { 0.0f, 0.0f, -1.0f }, // back

                                      { 0.0f, 1.0f, 0.0f },
                                      { 0.0f, 1.0f, 0.0f },
                                      { 0.0f, 1.0f, 0.0f },
                                      { 0.0f, 1.0f, 0.0f }, // top

                                      { 0.0f, -1.0f, 0.0f },
                                      { 0.0f, -1.0f, 0.0f },
                                      { 0.0f, -1.0f, 0.0f },
                                      { 0.0f, -1.0f, 0.0f }, // bottom

                                      { 1.0f, 0.0f, 0.0f },
                                      { 1.0f, 0.0f, 0.0f },
                                      { 1.0f, 0.0f, 0.0f },
                                      { 1.0f, 0.0f, 0.0f }, // right

                                      { -1.0f, 0.0f, 0.0f },
                                      { -1.0f, 0.0f, 0.0f },
                                      { -1.0f, 0.0f, 0.0f }, // left
                                      { -1.0f, 0.0f, 0.0f } };

    std::vector<glm::vec2> texcoord = { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f } };

    vertexContainers_.reserve(1);
    vertexContainers_.emplace_back();

    for (int i = 0; i < CUBE_VERTEX_COUNT; i++) {

        vertexContainers_.back().emplace_back(position[i], normal[i], color, texcoord[i % 4], glm::vec3(0.0f));
    }

    indexContainers_.reserve(1);
    indexContainers_.emplace_back();
    indexContainers_.back() = { 0, 1, 2, 2, 3, 0, 5, 4, 7, 7, 6, 5, 8, 9, 10, 10, 11, 8,
                                12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20 };

    name_ = "cube";

    meshParts_.reserve(1);
    meshParts_.emplace_back(0, -1);
}

void Mesh::CreateSphere(float scale, glm::vec3 color, const char* name, const char* texture)
{
    vertexContainers_.reserve(1);
    vertexContainers_.emplace_back();

    int division = 32;
    float degree = 360.0f / (float)division;

    glm::vec3 startPos{ 0.0f, -1.0f, 0.0f };
    glm::vec3 center{ 0.0f, 0.0f, 0.0f };

    startPos *= scale;
    center *= scale;

    glm::mat4 rotateZ;
    glm::vec3 basePosZ;
    float texU = 0.0f;
    float texV = 0.0f;
    int i;
    for (i = 0; i <= division / 2; i++) {
        rotateZ = glm::rotate(glm::mat4(1.0f), glm::radians(degree * i), glm::vec3(0.0f, 0.0f, 1.0f));
        basePosZ = glm::vec4(startPos, 1.0f) * rotateZ;
        texV = 1.0f - 1.0f / ((float)division / 2) * (float)i;
        for (int j = 0; j <= division; j++) {
            glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), glm::radians(degree * j), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::vec3 rotatePosY = glm::vec4(basePosZ, 1.0f) * rotateY;
            texU = 1.0f - 1.0f / ((float)division) * (float)j;
            vertexContainers_.back().emplace_back(rotatePosY, glm::normalize(glm::vec3(rotatePosY) - center), color, glm::vec2(texU, texV), glm::vec3(0.0f));
        }
    }

    indexContainers_.reserve(1);
    indexContainers_.emplace_back();

    for (int j = 0; j < division / 2; j++) {
        for (int i = division * j; i < division * (j + 1); i++) {
            indexContainers_.back().push_back(i + j);
            indexContainers_.back().push_back(i + j + 2 + division);
            indexContainers_.back().push_back(i + j + 1);

            indexContainers_.back().push_back(i + j + 2 + division);
            indexContainers_.back().push_back(i + j);
            indexContainers_.back().push_back(i + j + 1 + division);
        }
    }

    if (name)
        name_ = name;
    else
        name_ = "sphere";

    meshParts_.reserve(1);
    meshParts_.emplace_back(0, -1);
}

void Mesh::LoadModel(const std::string& modelPath, const char* texturePath, glm::vec3 color)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(modelPath.c_str(),
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
            materials_.emplace_back();
            aiString path;
            scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
            materials_.back().diffusePath = path.C_Str();
            scene->mMaterials[m]->GetTexture(aiTextureType_NORMALS, 0, &path);
            materials_.back().normalPath = path.C_Str();
        }
    }

    name_ = modelPath.substr(modelPath.rfind('/') + 1, modelPath.rfind('.') - modelPath.rfind('/') - 1);
}

void Mesh::ProcessNode(aiNode* node, const aiScene* scene)
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
}

void Mesh::ProcessLoadedMesh(aiMesh* mesh, glm::mat4& modelMat)
{
    vertexContainers_.emplace_back();
    indexContainers_.emplace_back();

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
        if (mesh->HasTangentsAndBitangents())
            tangent = { mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z };
        vertexContainers_.back().emplace_back(glm::vec3(pos.x, pos.y, pos.z),
                                              normal,
                                              glm::vec3(0.5f),
                                              texcoord,
                                              tangent);
    }

    meshParts_.emplace_back(meshParts_.size(), mesh->mMaterialIndex);
}

void Mesh::CreateBuffers()
{
    for (uint32_t i = 0; i < vertexContainers_.size(); i++) {

        CreateVertexBuffers(vertexContainers_[i]);
        CreateIndexBuffers(indexContainers_[i]);

        vkn::Command::Begin(commandBuffer_);
        // Copy vertices from staging buffer
        vkn::Command::CopyBufferToBuffer(commandBuffer_,
                                         vertexStagingBuffers.back()->GetBundle().buffer,
                                         vertexBuffers.back()->GetBundle().buffer,
                                         vertexStagingBuffers.back()->GetBufferInput().size);

        // Copy indices from staging buffer
        vkn::Command::CopyBufferToBuffer(commandBuffer_,
                                         indexStagingBuffers.back()->GetBundle().buffer,
                                         indexBuffers.back()->GetBundle().buffer,
                                         indexStagingBuffers.back()->GetBufferInput().size);

        commandBuffer_.end();
        vkn::Command::Submit(&commandBuffer_, 1);

        vertexStagingBuffers.back()->Destroy();
        indexStagingBuffers.back()->Destroy();
    }
}
