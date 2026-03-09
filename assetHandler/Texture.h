//
// Created by d on 2/13/26.
//

#ifndef TEXTURE_H
#define TEXTURE_H
#include <glm/vec3.hpp>
#include <stb/stb_image.h>

#include "vkRenderer/Image.h"

namespace Plunksna {

struct Texture
{
    //host
    stbi_uc* pixels = nullptr;
    glm::ivec3 extents;

    //device
    Image image;
    VkImageView fullView = VK_NULL_HANDLE;
    i32 mipLevels = 1;

    bool isHostLoaded() const
    {
        return pixels;
    }

    bool isDeviceLoaded() const
    {
        return image.image != VK_NULL_HANDLE;
    }

    i32 width() const
    {
        return extents.x;
    }

    i32 height() const
    {
        return extents.y;
    }

    i32 depth() const
    {
        return extents.z;
    }

    VkDeviceSize getSize() const
    {
        return extents.x * extents.y;
    }
};

}

#endif //TEXTURE_H
