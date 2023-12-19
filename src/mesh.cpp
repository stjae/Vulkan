#include "mesh.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

void Mesh::CreateSquare()
{
    const int SQUARE_VERTEX_COUNT = 4;

    std::vector<float> pos = { -1.0f, 1.0f, 0.0f,
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

        Vertex v;

        v.pos = glm::vec3(pos[i * 3 + 0], pos[i * 3 + 1], pos[i * 3 + 2]);
        v.normal = glm::vec3(normal[i * 3 + 0], normal[i * 3 + 1], normal[i * 3 + 2]);
        v.texcoord = glm::vec2(texcoord[i * 2], texcoord[i * 2 + 1]);
        v.textureID = 0;

        vertices.push_back(v);
    }

    indices = { 0, 1, 2, 2, 3, 0 };
}

void Mesh::CreateCube(const char* texturePath)
{
    textureFilePath = texturePath;

    const int CUBE_VERTEX_COUNT = 24;

    std::vector<float> pos = { -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
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
                               -1.0f, -1.0f, 1.0f, // right
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

        Vertex v;

        v.pos = glm::vec3(pos[i * 3 + 0], pos[i * 3 + 1], pos[i * 3 + 2]);
        v.normal = glm::vec3(normal[i * 3 + 0], normal[i * 3 + 1], normal[i * 3 + 2]);
        v.texcoord = glm::vec2(texcoord[i * 2 + 0], texcoord[i * 2 + 1]);
        v.textureID = 0;

        vertices.push_back(v);
    }

    indices = { 0, 1, 2, 2, 3, 0, 5, 4, 7, 7, 6, 5, 8, 9, 10, 10, 11, 8,
                12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20 };

    name = "cube";
}

void Mesh::LoadModel(const char* modelPath, const char* texturePath)
{
    textureFilePath = texturePath;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string filePath(PROJECT_DIR);
    filePath.append(modelPath);

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    vertices.reserve(shapes.size() * shapes[0].mesh.indices.size());

    for (auto& shape : shapes) {
        for (auto& index : shape.mesh.indices) {

            Vertex v;

            v.pos = glm::vec3(attrib.vertices[3 * index.vertex_index + 0],
                              attrib.vertices[3 * index.vertex_index + 1],
                              attrib.vertices[3 * index.vertex_index + 2]);

            v.normal = glm::vec3(attrib.normals[3 * index.normal_index + 0],
                                 attrib.normals[3 * index.normal_index + 1],
                                 attrib.normals[3 * index.normal_index + 2]);

            v.texcoord = glm::vec2(attrib.texcoords[2 * index.texcoord_index + 0],
                                   1.0f - attrib.texcoords[2 * index.texcoord_index + 1]);

            v.textureID = 1;

            vertices.push_back(v);

            indices.push_back(3 * index.vertex_index + 2);
            indices.push_back(3 * index.vertex_index + 1);
            indices.push_back(3 * index.vertex_index + 0);
        }
    }

    name = "model";
}