//
// Created by d on 2/13/26.
//

#ifndef TEXTURE_H
#define TEXTURE_H
#include <glm/vec3.hpp>
#include <stb/stb_image.h>

#include "vkrenderer/Image.h"

namespace Plunksna {

struct Texture
{
    //host
    stbi_uc* pixels = nullptr;
    glm::ivec3 extents;

    //device
    Image image;
    VkImageView fullView = VK_NULL_HANDLE;
    uint32_t mipLevels = 1;

    bool isHostLoaded() const
    {
        return pixels;
    }

    bool isDeviceLoaded() const
    {
        return image.image != VK_NULL_HANDLE;
    }

    uint32_t width() const
    {
        return extents.x;
    }

    uint32_t height() const
    {
        return extents.y;
    }

    uint32_t depth() const
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
