#include "mesh.h"

void Mesh::AddInstance(glm::vec3 pos, glm::vec3 scale)
{
    const uint64_t UUID = ID::GenerateID();
    m_meshInstances.push_back(std::make_unique<MeshInstance>(UUID, MeshInstanceUBO(m_meshColorID, (int32_t)m_meshInstances.size(), pos, scale)));
    m_meshInstanceUBOs.push_back(m_meshInstances.back()->UBO);
}

void Mesh::AddInstance(const uint64_t UUID)
{
    m_meshInstances.push_back(std::make_unique<MeshInstance>(UUID, MeshInstanceUBO(m_meshColorID, (int32_t)m_meshInstances.size())));
    m_meshInstanceUBOs.push_back(m_meshInstances.back()->UBO);
}

void Mesh::DeleteInstance(int32_t instanceColorID)
{
    m_meshInstances.erase(m_meshInstances.begin() + instanceColorID);
    for (int32_t i = instanceColorID; i < m_meshInstances.size(); i++) {
        m_meshInstances[i]->UBO.instanceColorID--;
    }
}

void Mesh::CreateSquare(float scale, const char* texturePath)
{
    const int SQUARE_VERTEX_COUNT = 4;

    std::vector<glm::vec3> position = { { -1.0f, 1.0f, 0.0f },
                                        { -1.0f, -1.0f, 0.0f },
                                        { 1.0f, -1.0f, 0.0f },
                                        { 1.0f, 1.0f, 0.0f } };

    std::vector<glm::vec3> normal = { { 0.0f, 1.0f, 0.0f },
                                      { 0.0f, 1.0f, 0.0f },
                                      { 0.0f, 1.0f, 0.0f },
                                      { 0.0f, 1.0f, 0.0f } };

    std::vector<glm::vec2> texcoord = { { 0.0f, 0.0f },
                                        { 0.0f, 1.0f },
                                        { 1.0f, 1.0f },
                                        { 1.0f, 0.0f } };

    m_vertexContainers.reserve(1);
    m_vertexContainers.emplace_back();

    for (int i = 0; i < SQUARE_VERTEX_COUNT; i++) {
        m_vertexContainers.back().emplace_back(position[i] * scale, normal[i], texcoord[i]);
    }

    m_indexContainers.reserve(1);
    m_indexContainers.emplace_back();
    m_indexContainers.back() = { 0, 1, 2, 2, 3, 0 };

    m_name = "square";

    m_meshParts.reserve(1);
    m_meshParts.emplace_back(0, -1);
}

void Mesh::CreateCube(float scale, const char* texturePath)
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

    m_vertexContainers.reserve(1);
    m_vertexContainers.emplace_back();

    for (int i = 0; i < CUBE_VERTEX_COUNT; i++) {
        m_vertexContainers.back().emplace_back(position[i] * scale, normal[i], texcoord[i % 4]);
    }

    m_indexContainers.reserve(1);
    m_indexContainers.emplace_back();
    m_indexContainers.back() = { 0, 1, 2, 2, 3, 0, 5, 4, 7, 7, 6, 5, 8, 9, 10, 10, 11, 8,
                                 12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20 };

    m_name = "cube";

    m_meshParts.reserve(1);
    m_meshParts.emplace_back(0, -1);
}

void Mesh::CreateSphere(float scale, const char* name, const char* texture)
{
    m_vertexContainers.reserve(1);
    m_vertexContainers.emplace_back();

    int division = 128;
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
            m_vertexContainers.back().emplace_back(rotatePosY, glm::normalize(glm::vec3(rotatePosY) - center), glm::vec2(texU, texV));
        }
    }

    m_indexContainers.reserve(1);
    m_indexContainers.emplace_back();

    for (int j = 0; j < division / 2; j++) {
        for (int i = division * j; i < division * (j + 1); i++) {
            m_indexContainers.back().push_back(i + j);
            m_indexContainers.back().push_back(i + j + 2 + division);
            m_indexContainers.back().push_back(i + j + 1);

            m_indexContainers.back().push_back(i + j + 2 + division);
            m_indexContainers.back().push_back(i + j);
            m_indexContainers.back().push_back(i + j + 1 + division);
        }
    }

    if (name)
        m_name = name;
    else
        m_name = "sphere";

    m_meshParts.reserve(1);
    m_meshParts.emplace_back(0, -1);
}

Mesh::Mesh(int meshColorID, const std::string& filePath, const std::string& fileName) : m_meshColorID(meshColorID)
{
    if (!filePath.empty()) {
        m_filePath = filePath;
        m_name = fileName;
        LoadModel(filePath);
    }
}

void Mesh::LoadModel(const std::string& filepath)
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
        m_materials.reserve(scene->mNumMaterials);
        for (unsigned int m = 0; m < scene->mNumMaterials; m++) {
            aiString path;
            m_materials.emplace_back();
            scene->mMaterials[m]->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &path);
            m_materials.back().albedo = path.C_Str();
            scene->mMaterials[m]->GetTexture(aiTextureType_NORMALS, 0, &path);
            m_materials.back().normal = path.C_Str();
            scene->mMaterials[m]->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &path);
            m_materials.back().metallic = path.C_Str();
            scene->mMaterials[m]->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &path);
            m_materials.back().roughness = path.C_Str();
        }
    }
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
    for (auto& bulletMesh : m_bulletMeshes) {
        m_bulletVertexArray.addIndexedMesh(bulletMesh);
    }
}

void Mesh::ProcessLoadedMesh(aiMesh* mesh, glm::mat4& modelMat)
{
    m_vertexContainers.emplace_back();
    m_indexContainers.emplace_back();
    m_posContainers.emplace_back();

    aiFace* faces = mesh->mFaces;
    for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
        unsigned int* indices = faces[f].mIndices;
        m_indexContainers.back().push_back(indices[0]);
        m_indexContainers.back().push_back(indices[1]);
        m_indexContainers.back().push_back(indices[2]);
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
        m_vertexContainers.back().emplace_back(glm::vec3(pos.x, pos.y, pos.z),
                                               normal,
                                               texcoord,
                                               tangent,
                                               biTangent);

        m_posContainers.back().push_back(pos.x);
        m_posContainers.back().push_back(pos.y);
        m_posContainers.back().push_back(pos.z);
    }

    m_meshParts.emplace_back(m_meshParts.size(), mesh->mMaterialIndex);

    m_bulletMeshes.emplace_back();
    m_bulletMeshes.back().m_triangleIndexBase = (unsigned char*)m_indexContainers.back().data();
    m_bulletMeshes.back().m_triangleIndexStride = 3 * sizeof(unsigned int);
    m_bulletMeshes.back().m_vertexBase = (unsigned char*)m_posContainers.back().data();
    m_bulletMeshes.back().m_vertexStride = 3 * sizeof(float);
    m_bulletMeshes.back().m_numTriangles = (int)mesh->mNumFaces;
    m_bulletMeshes.back().m_numVertices = (int)mesh->mNumVertices;
}

void Mesh::CreateBuffers(const vk::CommandBuffer& commandBuffer)
{
    for (uint32_t i = 0; i < m_vertexContainers.size(); i++) {

        CreateVertexBuffers(m_vertexContainers[i]);
        CreateIndexBuffers(m_indexContainers[i]);

        vkn::Command::CopyBufferToBuffer(commandBuffer, m_vertexStagingBuffers.back()->Get().buffer, m_vertexBuffers.back()->Get().buffer, m_vertexStagingBuffers.back()->Get().bufferInfo.size);
        vkn::Command::CopyBufferToBuffer(commandBuffer, m_indexStagingBuffers.back()->Get().buffer, m_indexBuffers.back()->Get().buffer, m_indexStagingBuffers.back()->Get().bufferInfo.size);
    }
}

void Mesh::UpdateUBO(MeshInstance& instance)
{
    m_meshInstanceUBOs[instance.UBO.instanceColorID] = instance.UBO;
    m_meshInstanceUBOBuffer->Copy(m_meshInstanceUBOs.data());
    instance.physicsDebugUBO.model = instance.UBO.model;
    if (instance.camera)
        instance.camera->GetTranslation() = instance.translation;
}

void Mesh::UpdateColorID()
{
    for (auto& instance : m_meshInstances) {
        instance->UBO.meshColorID = m_meshColorID;
        UpdateUBO(*instance);
    }
}

MeshInstanceUBO::MeshInstanceUBO(int32_t meshColorID, int32_t instanceColorID, glm::vec3 pos, glm::vec3 scale)
    : meshColorID(meshColorID), instanceColorID(instanceColorID)
{
    model = glm::translate(model, pos);
    model = glm::scale(model, scale);
}

MeshInstance::MeshInstance(uint64_t UUID, MeshInstanceUBO&& UBO)
    : UUID(UUID), UBO(UBO)
{
    vkn::BufferInfo bufferInfo = { sizeof(PhysicsDebugUBO), vk::WholeSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    physicsDebugUBOBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
}

MeshInstance& MeshInstance::operator=(const MeshInstance& other)
{
    if (&other == this)
        return *this;
    this->translation = other.translation;
    this->rotation = other.rotation;
    this->scale = other.scale;
    this->UBO = other.UBO;
    this->physicsDebugUBO.model = UBO.model;
    return *this;
}

void MeshInstance::UpdateTransform()
{
    // update translation, rotation, scale based on model matrix
    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(UBO.model), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));
}

void MeshInstance::UpdateMatrix()
{
    // update matrix based on translation, rotation, scale
    ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale), glm::value_ptr(UBO.model));
}
