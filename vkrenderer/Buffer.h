//
// Created by d on 2/12/26.
//

#ifndef BUFFER_H
#define BUFFER_H

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include "Context.h"

namespace Plunksna {

struct Buffer
{
    VkBuffer buffer          = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;

    void destroy(const Context& context)
    {
        vmaDestroyBuffer(context.allocator, buffer, allocation);
        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
    }
};

}

#endif //BUFFER_H
