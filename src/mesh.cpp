#include "mesh.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

void Mesh::CreateSquare(const char* texturePath)
{
    textureFilePath = texturePath;

    const int SQUARE_VERTEX_COUNT = 4;

    std::vector<float> position = { -1.0f, 1.0f, 0.0f,
                                    1.0f, 1.0f, 0.0f,
                                    1.0f, -1.0f, 0.0f,
                                    -1.0f, -1.0f, 0.0f };

    std::vector<float> normal = { 1.0f, 0.0f, 0.0f,
                                  0.0f, 1.0f, 0.0f,
                                  0.0f, 0.0f, 1.0f,
                                  1.0f, 1.0f, 1.0f };

    std::vector<float> texcoord = { 0.0f, 0.0f,
                                    1.0f, 0.0f,
                                    1.0f, 1.0f,
                                    0.0f, 1.0f };

    vertices.reserve(SQUARE_VERTEX_COUNT);

    for (int i = 0; i < SQUARE_VERTEX_COUNT; i++) {

        Vertex v{};

        v.pos = glm::vec3(position[i * 3 + 0], position[i * 3 + 1], position[i * 3 + 2]);
        v.normal = glm::vec3(normal[i * 3 + 0], normal[i * 3 + 1], normal[i * 3 + 2]);
        v.texcoord = glm::vec2(texcoord[i * 2], texcoord[i * 2 + 1]);
        v.textureID = 0;

        vertices.push_back(v);
    }

    indices = { 0, 1, 2, 2, 3, 0 };

    name_ = "square";
}

void Mesh::CreateCube(const char* texturePath)
{
    textureFilePath = texturePath;

    const int CUBE_VERTEX_COUNT = 24;

    std::vector<float> position = { -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                                    1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, // front

                                    -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
                                    1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, // back

                                    -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
                                    1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, // ceiling

                                    -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
                                    1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, // bottom

                                    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
                                    1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, // left

                                    -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
                                    -1.0f, -1.0f, 1.0f, // right_
                                    -1.0f, -1.0f, -1.0f };

    std::vector<float> normal = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                                  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

                                  0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
                                  0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,

                                  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

                                  0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
                                  0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,

                                  -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
                                  -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

                                  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };

    std::vector<float> texcoord = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,

                                    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,

                                    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,

                                    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,

                                    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,

                                    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };

    vertices.reserve(CUBE_VERTEX_COUNT);

    for (int i = 0; i < CUBE_VERTEX_COUNT; i++) {

        Vertex v{};

        v.pos = glm::vec3(position[i * 3 + 0], position[i * 3 + 1], position[i * 3 + 2]);
        v.normal = glm::vec3(normal[i * 3 + 0], normal[i * 3 + 1], normal[i * 3 + 2]);
        v.texcoord = glm::vec2(texcoord[i * 2 + 0], texcoord[i * 2 + 1]);
        v.textureID = 0;

        vertices.push_back(v);
    }

    indices = { 0, 1, 2, 2, 3, 0, 5, 4, 7, 7, 6, 5, 8, 9, 10, 10, 11, 8,
                12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20 };

    name_ = "cube";
}

void Mesh::LoadModel(const std::string& modelPath, const char* texturePath)
{
    textureFilePath = texturePath;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    // Loop over shapes
    for (auto & shape : shapes) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            auto fv = size_t(shape.mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                Vertex vertex{};
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                indices.push_back(index_offset + v);

                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                vertex.pos = glm::vec3(vx, vy, vz);

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    vertex.normal = glm::vec3(nx, ny, nz);
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    vertex.texcoord = glm::vec2(tx, ty);
                }
                vertex.textureID = 0;

                vertices.push_back(vertex);
            }
            index_offset += fv;
        }
    }

    name_ = modelPath.substr(modelPath.rfind('/') + 1, modelPath.rfind('.') - modelPath.rfind('/') - 1);
}

void Mesh::CreateBuffers()
{
    CreateIndexBuffer();
    CreateVertexBuffer();

    CreateTexture();
    textureImage->CreateSampler();
}