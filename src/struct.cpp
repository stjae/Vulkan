#include "struct.h"

MeshInstanceUBO::MeshInstanceUBO(int32_t meshColorID, int32_t instanceColorID, glm::vec3 pos, glm::vec3 scale) : model(1.0f), invTranspose(1.0f), meshColorID(meshColorID), textureID(0), useTexture(true), instanceColorID(instanceColorID), albedo({ 0.5, 0.5, 0.5 }), metallic(0.0f), roughness(1.0f)
{
    model = glm::translate(model, pos);
    model = glm::scale(model, scale);
    invTranspose = model;
    invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    invTranspose = glm::transpose(glm::inverse(invTranspose));
}
