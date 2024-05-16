#include "mesh.h"

void Mesh::AddInstance(glm::vec3 pos, glm::vec3 scale)
{
    m_meshInstanceUBOs.emplace_back(m_meshID, m_meshInstanceUBOs.size(), pos, scale);
}

void Mesh::AddPhysicsInfo(const MeshPhysicsInfo& pInfo)
{
    m_physicsInfo = std::make_unique<MeshPhysicsInfo>(pInfo);
    m_physicsDebugDrawer = std::make_unique<PhysicsDebugDrawer>(pInfo, m_indexContainers, m_vertexContainers);
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

void Mesh::CreateBuffers(const vk::CommandBuffer& commandBuffer)
{
    for (uint32_t i = 0; i < m_vertexContainers.size(); i++) {

        CreateVertexBuffers(m_vertexContainers[i]);
        CreateIndexBuffers(m_indexContainers[i]);

        vkn::Command::Begin(commandBuffer);
        // Copy vertices from staging buffer
        vkn::Command::CopyBufferToBuffer(commandBuffer,
                                         m_vertexStagingBuffers.back()->Get().buffer,
                                         m_vertexBuffers.back()->Get().buffer,
                                         m_vertexStagingBuffers.back()->Get().bufferInfo.size);

        // Copy indices from staging buffer
        vkn::Command::CopyBufferToBuffer(commandBuffer,
                                         m_indexStagingBuffers.back()->Get().buffer,
                                         m_indexBuffers.back()->Get().buffer,
                                         m_indexStagingBuffers.back()->Get().bufferInfo.size);

        commandBuffer.end();

        vkn::Command::SubmitAndWait(commandBuffer);

        m_vertexStagingBuffers.back()->Destroy();
        m_indexStagingBuffers.back()->Destroy();
    }
}
