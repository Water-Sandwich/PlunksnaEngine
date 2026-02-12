//
// Created by d on 2/12/26.
//

#ifndef IMAGE_H
#define IMAGE_H
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include "Context.h"

namespace Plunksna {

struct Image
{
    VkImage image            = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;

    void destroy(const Context& context)
    {
        vmaDestroyImage(context.allocator, image, allocation);
        image = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
    }
};

}

#endif //IMAGE_H
