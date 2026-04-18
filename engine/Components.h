//
// Created by d on 6/27/25.
//

#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "assetHandler/Asset.h"
#include <glm/mat4x4.hpp>

namespace Plunksna {

struct Model
{
    Asset mesh;
    Asset texture;
};

struct Sine
{
    glm::mat4 tempTx;
    f32 speed;
    f32 amplitude;
    f32 timer = 0;
};

using Transform3D = glm::mat4;

}

#endif //COMPONENTS_H
