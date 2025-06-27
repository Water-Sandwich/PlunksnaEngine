//
// Created by d on 6/27/25.
//

#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <glm/vec2.hpp>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>

namespace Plunksna {

struct Transform2
{
    glm::vec2 position, scale;
    float rotation;

    Transform2() noexcept : position(0,0), scale(1,1), rotation(0){}
    Transform2(glm::vec2 pos) noexcept : position(pos), scale(1,1), rotation(0){}
    Transform2(glm::vec2 pos, glm::vec2 scl) noexcept : position(pos), scale(scl), rotation(0){}
    Transform2(glm::vec2 pos, float rot) noexcept : position(pos), scale(1,1), rotation(rot){}
    Transform2(glm::vec2 pos, glm::vec2 scl, float rot) noexcept : position(pos), scale(scl), rotation(rot){}
};

struct RColorRGBA
{
    Uint8 r,g,b,a;
};

struct RTexture
{
    SDL_Texture* texture;
};

}

#endif //COMPONENTS_H
