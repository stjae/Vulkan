#include "mesh.h"

void Mesh::AddInstance(glm::vec3 pos, glm::vec3 scale)
{
    meshInstanceUBOs_.emplace_back(meshID_, meshInstanceUBOs_.size(), pos, scale);
    meshInstancePhysicsInfos_.emplace_back();
}

void Mesh::CreateSquare(float scale, const char* texturePath)
{
    const int SQUARE_VERTEX_COUNT = 4;

    // std::vector<glm::vec3> position = { { -1.0f, 0.0f, -1.0f },
    //                                     { -1.0f, 0.0f, 1.0f },
    //                                     { 1.0f, 0.0f, 1.0f },
    //                                     { 1.0f, 0.0f, -1.0f } };
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

    vertexContainers_.reserve(1);
    vertexContainers_.emplace_back();

    for (int i = 0; i < SQUARE_VERTEX_COUNT; i++) {

        position[i] *= scale;
        vertexContainers_.back().emplace_back(position[i], normal[i], texcoord[i]);
    }

    indexContainers_.reserve(1);
    indexContainers_.emplace_back();
    indexContainers_.back() = { 0, 1, 2, 2, 3, 0 };

    name_ = "square";

    meshParts_.reserve(1);
    meshParts_.emplace_back(0, -1);
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

    vertexContainers_.reserve(1);
    vertexContainers_.emplace_back();

    for (int i = 0; i < CUBE_VERTEX_COUNT; i++) {

        vertexContainers_.back().emplace_back(position[i], normal[i], texcoord[i % 4]);
    }

    indexContainers_.reserve(1);
    indexContainers_.emplace_back();
    indexContainers_.back() = { 0, 1, 2, 2, 3, 0, 5, 4, 7, 7, 6, 5, 8, 9, 10, 10, 11, 8,
                                12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20 };

    name_ = "cube";

    meshParts_.reserve(1);
    meshParts_.emplace_back(0, -1);
}

void Mesh::CreateSphere(float scale, const char* name, const char* texture)
{
    vertexContainers_.reserve(1);
    vertexContainers_.emplace_back();

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
            vertexContainers_.back().emplace_back(rotatePosY, glm::normalize(glm::vec3(rotatePosY) - center), glm::vec2(texU, texV));
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
