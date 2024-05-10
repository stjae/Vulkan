#ifndef ENUM_H
#define ENUM_H

enum class eRigidBodyType {
    STATIC,
    DYNAMIC
};

enum class eColliderShape {
    BOX,
    SPHERE,
    CAPSULE,
    CYLINDER,
    CONE,
    MESH
};

#endif