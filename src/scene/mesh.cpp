#include "mesh.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

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

    vertices.reserve(SQUARE_VERTEX_COUNT);

    glm::vec3 tangent(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < SQUARE_VERTEX_COUNT; i++) {

        position[i] *= scale;
        vertices.emplace_back(position[i], normal[i], color, texcoord[i], tangent);
    }

    indices = { 0, 1, 2, 2, 3, 0 };

    name_ = "square";

    meshParts_.reserve(1);
    meshParts_.emplace_back(-1, indices.size(), 0);
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

    vertices.reserve(CUBE_VERTEX_COUNT);

    glm::vec3 tangent(0.0f);
    for (int i = 0; i < CUBE_VERTEX_COUNT; i++) {

        vertices.emplace_back(position[i], normal[i], color, texcoord[i % 4], tangent);
    }

    indices = { 0, 1, 2, 2, 3, 0, 5, 4, 7, 7, 6, 5, 8, 9, 10, 10, 11, 8,
                12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20 };

    name_ = "cube";

    meshParts_.reserve(1);
    meshParts_.emplace_back(-1, indices.size(), 0);
}

void Mesh::CreateSphere(float scale, glm::vec3 color, const char* name, const char* texture)
{
    int division = 32;
    float degree = 360.0f / (float)division;

    glm::vec3 startPos{ 0.0f, -1.0f, 0.0f };
    glm::vec3 center{ 0.0f, 0.0f, 0.0f };
    glm::vec3 tangent(0.0f);

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
            vertices.emplace_back(rotatePosY, glm::normalize(glm::vec3(rotatePosY) - center), color, glm::vec2(texU, texV), tangent);
        }
    }

    for (int j = 0; j < division / 2; j++) {
        for (int i = division * j; i < division * (j + 1); i++) {
            indices.push_back(i + j);
            indices.push_back(i + j + 2 + division);
            indices.push_back(i + j + 1);

            indices.push_back(i + j + 2 + division);
            indices.push_back(i + j);
            indices.push_back(i + j + 1 + division);
        }
    }

    if (name)
        name_ = name;
    else
        name_ = "sphere";

    meshParts_.reserve(1);
    meshParts_.emplace_back(-1, indices.size(), 0);
}

void Mesh::LoadModel(const std::string& modelPath, const char* texturePath, glm::vec3 color)
{
    tinyobj::ObjReaderConfig reader_config;
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(modelPath, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    bool hasNormal = false;

    meshParts_.resize(shapes.size(), { -1, 0, 0 });
    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        meshParts_[s].indexBase = indices.size();

        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                Vertex vertex;
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                vertex.pos = glm::vec3(vx, vy, vz);

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    hasNormal = true;
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    vertex.normal = glm::vec3(nx, ny, nz);
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = 1.0f - attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    vertex.texcoord = glm::vec2(tx, ty);
                }

                // Optional: vertex colors
                // tinyobj::real_t red = attrib.colors[3 * size_t(idx.vertex_index) + 0];
                // tinyobj::real_t green = attrib.colors[3 * size_t(idx.vertex_index) + 1];
                // tinyobj::real_t blue = attrib.colors[3 * size_t(idx.vertex_index) + 2];
                vertex.color = color;

                vertices.push_back(vertex);
                indices.push_back(indices.size());
            }
            if (!hasNormal) {
                glm::vec3 vertex2 = vertices[vertices.size() - 1].pos;
                glm::vec3 vertex1 = vertices[vertices.size() - 2].pos;
                glm::vec3 vertex0 = vertices[vertices.size() - 3].pos;

                glm::vec3 x1 = vertex1 - vertex0;
                glm::vec3 x2 = vertex2 - vertex0;
                vertices[vertices.size() - 3].normal = glm::normalize(glm::cross(x1, x2));

                glm::vec3 y1 = vertex2 - vertex1;
                glm::vec3 y2 = vertex0 - vertex1;
                vertices[vertices.size() - 2].normal = glm::normalize(glm::cross(y1, y2));

                glm::vec3 z1 = vertex0 - vertex2;
                glm::vec3 z2 = vertex1 - vertex2;
                vertices[vertices.size() - 1].normal = glm::normalize(glm::cross(z1, z2));
            }
            index_offset += fv;

            // per-face material
            // materials[shapes[s].mesh.material_ids[f]];
            meshParts_[s].materialID = shapes[s].mesh.material_ids[f];
        }

        meshParts_[s].indexCount = indices.size() - meshParts_[s].indexBase;
    }

    materials_ = materials;
    name_ = modelPath.substr(modelPath.rfind('/') + 1, modelPath.rfind('.') - modelPath.rfind('/') - 1);
}

void Mesh::CreateBuffers()
{
    CreateIndexBuffer();
    CreateVertexBuffer();

    Command::Begin(commandBuffer_);
    // Copy vertices from staging buffer
    Command::CopyBufferToBuffer(commandBuffer_,
                                vertexStagingBuffer->GetBundle().buffer,
                                vertexBuffer->GetBundle().buffer,
                                vertexStagingBuffer->GetBufferInput().size);

    // Copy indices from staging buffer
    Command::CopyBufferToBuffer(commandBuffer_,
                                indexStagingBuffer->GetBundle().buffer,
                                indexBuffer->GetBundle().buffer,
                                indexStagingBuffer->GetBufferInput().size);

    commandBuffer_.end();
    Command::Submit(&commandBuffer_, 1);

    vertexStagingBuffer->Destroy();
    indexStagingBuffer->Destroy();
}
