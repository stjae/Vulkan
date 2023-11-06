#include "mesh.h"

void Mesh::CreateSquare()
{
    std::vector<float> pos = { -1.0f, 1.0f, 0.0f,
                               1.0f, 1.0f, 0.0f,
                               1.0f, -1.0f, 0.0f,
                               -1.0f, -1.0f, 0.0f };

    std::vector<float> color = { 1.0f, 0.0f, 0.0f,
                                 0.0f, 1.0f, 0.0f,
                                 0.0f, 0.0f, 1.0f,
                                 1.0f, 1.0f, 1.0f };

    vertices.reserve(pos.size() + color.size());
    // vertices.reserve(pos.size() + pos.size());

    for (int i = 0; i < pos.size(); i += 3) {

        for (int j = i; j < i + 3; ++j) {
            vertices.push_back(pos[j]);
        }

        for (int j = i; j < i + 3; ++j) {
            vertices.push_back(color[j]);
            // vertices.push_back(1.0f);
        }
    }

    indices = { 0, 1, 2, 2, 3, 0 };
}

void Mesh::CreateCube()
{
    std::vector<float> pos = { -1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f,
                               1.0f, -1.0f, 1.0f,
                               -1.0f, -1.0f, 1.0f,

                               -1.0f, 1.0f, -1.0f,
                               1.0f, 1.0f, -1.0f,
                               1.0f, -1.0f, -1.0f,
                               -1.0f, -1.0f, -1.0f };

    std::vector<float> color = { 1.0f, 0.0f, 0.0f,
                                 0.0f, 1.0f, 0.0f,
                                 0.0f, 0.0f, 1.0f,
                                 1.0f, 1.0f, 1.0f,

                                 1.0f, 0.0f, 0.0f,
                                 0.0f, 1.0f, 0.0f,
                                 0.0f, 0.0f, 1.0f,
                                 1.0f, 1.0f, 1.0f };

    vertices.reserve(pos.size() + color.size());

    for (int i = 0; i < pos.size(); i += 3) {

        for (int j = i; j < i + 3; ++j) {
            vertices.push_back(pos[j]);
        }

        for (int j = i; j < i + 3; ++j) {
            vertices.push_back(color[j]);
        }
    }

    indices = { 0, 1, 2, 2, 3, 0,
                5, 4, 7, 7, 6, 5,
                4, 5, 1, 1, 0, 4,
                3, 2, 6, 6, 7, 3,
                4, 0, 3, 3, 7, 4,
                1, 5, 6, 6, 2, 1 };
}
