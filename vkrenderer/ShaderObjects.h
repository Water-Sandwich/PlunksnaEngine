//
// Created by d on 2/23/26.
//

#ifndef SHADEROBJECTS_H
#define SHADEROBJECTS_H
#include <glm/glm.hpp>

namespace Plunksna {

struct CameraSO
{
    glm::mat4 view;
    glm::mat4 proj;
};

struct PerObjectSO
{
    glm::mat4 model;
    alignas(16) u32 textureIndex = 0;

    PerObjectSO() = default;
    PerObjectSO(const glm::mat4& mod)
    {
        model = mod;
    }
};

}

#endif //SHADEROBJECTS_H
