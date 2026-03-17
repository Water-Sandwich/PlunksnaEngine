//
// Created by d on 3/16/26.
//

#ifndef CULLSPHERE_H
#define CULLSPHERE_H
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

struct CullSphere
{
    glm::vec3 offset;
    glm::f32 radius;
};

#endif //CULLSPHERE_H
