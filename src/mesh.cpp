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
            pos[j] *= 1.5f;
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
                               -1.0f, -1.0f, 1.0f, // front

                               -1.0f, 1.0f, -1.0f,
                               1.0f, 1.0f, -1.0f,
                               1.0f, -1.0f, -1.0f,
                               -1.0f, -1.0f, -1.0f, // back

                               -1.0f, 1.0f, -1.0f,
                               1.0f, 1.0f, -1.0f,
                               1.0f, 1.0f, 1.0f,
                               -1.0f, 1.0f, 1.0f, // ceiling

                               -1.0f, -1.0f, 1.0f,
                               1.0f, -1.0f, 1.0f,
                               1.0f, -1.0f, -1.0f,
                               -1.0f, -1.0f, -1.0f, // bottom

                               1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, -1.0f,
                               1.0f, -1.0f, -1.0f,
                               1.0f, -1.0f, 1.0f, // left

                               -1.0f, 1.0f, -1.0f,
                               -1.0f, 1.0f, 1.0f,
                               -1.0f, -1.0f, 1.0f, // right
                               -1.0f, -1.0f, -1.0f };

    std::vector<float> normal = { 0.0f, 0.0f, 1.0f,
                                  0.0f, 0.0f, 1.0f,
                                  0.0f, 0.0f, 1.0f,
                                  0.0f, 0.0f, 1.0f,

                                  0.0f, 0.0f, -1.0f,
                                  0.0f, 0.0f, -1.0f,
                                  0.0f, 0.0f, -1.0f,
                                  0.0f, 0.0f, -1.0f,

                                  0.0f, 1.0f, 0.0f,
                                  0.0f, 1.0f, 0.0f,
                                  0.0f, 1.0f, 0.0f,
                                  0.0f, 1.0f, 0.0f,

                                  0.0f, -1.0f, 0.0f,
                                  0.0f, -1.0f, 0.0f,
                                  0.0f, -1.0f, 0.0f,
                                  0.0f, -1.0f, 0.0f,

                                  -1.0f, 0.0f, 0.0f,
                                  -1.0f, 0.0f, 0.0f,
                                  -1.0f, 0.0f, 0.0f,
                                  -1.0f, 0.0f, 0.0f,

                                  1.0f, 0.0f, 0.0f,
                                  1.0f, 0.0f, 0.0f,
                                  1.0f, 0.0f, 0.0f,
                                  1.0f, 0.0f, 0.0f };

    std::vector<float> texcoord = { 1.0f, 0.0f,
                                    0.0f, 0.0f,
                                    0.0f, 1.0f,
                                    1.0f, 1.0f,

                                    1.0f, 0.0f,
                                    0.0f, 0.0f,
                                    0.0f, 1.0f,
                                    1.0f, 1.0f,

                                    1.0f, 0.0f,
                                    0.0f, 0.0f,
                                    0.0f, 1.0f,
                                    1.0f, 1.0f,

                                    1.0f, 0.0f,
                                    0.0f, 0.0f,
                                    0.0f, 1.0f,
                                    1.0f, 1.0f,

                                    1.0f, 0.0f,
                                    0.0f, 0.0f,
                                    0.0f, 1.0f,
                                    1.0f, 1.0f,

                                    1.0f, 0.0f,
                                    0.0f, 0.0f,
                                    0.0f, 1.0f,
                                    1.0f, 1.0f };

    vertices.reserve(pos.size() + normal.size() + texcoord.size());

    int k = 0;
    for (int i = 0; i < pos.size(); i += 3) {

        for (int j = i; j < i + 3; ++j) {
            vertices.push_back(pos[j]);
        }

        for (int j = i; j < i + 3; ++j) {
            vertices.push_back(normal[j]);
        }

        for (int j = k; j < k + 2; ++j) {
            vertices.push_back(texcoord[j]);
        }

        k += 2;
    }

    indices = { 0, 1, 2, 2, 3, 0,
                5, 4, 7, 7, 6, 5,
                8, 9, 10, 10, 11, 8,
                12, 13, 14, 14, 15, 12,
                16, 17, 18, 18, 19, 16,
                20, 21, 22, 22, 23, 20 };
}
