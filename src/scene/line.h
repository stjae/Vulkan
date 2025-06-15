// LinePoint structure: Used for grid/physics debugging line drawing
// LinePoint構造体：グリッド／物理デバッグ用ライン描画で使用

#ifndef LINE_H
#define LINE_H

#include "glm/glm.hpp"
#include "bullet/btBulletCollisionCommon.h"

struct LinePoint
{
    glm::vec3 pos;
    glm::vec3 color;
    LinePoint(glm::vec3 pos, glm::vec3 color = { 1.0f, 1.0f, 1.0f }) : pos(pos), color(color) {}
    LinePoint(const btVector3& pos, const btVector3& color) : pos(pos.x(), pos.y(), pos.z()), color(color.x(), color.y(), color.z()) {}
};

#endif