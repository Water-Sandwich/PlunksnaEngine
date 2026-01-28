//
// Created by d on 1/28/26.
//

#ifndef CONTEXT_H
#define CONTEXT_H

#include <optional>
#include <vulkan/vulkan_core.h>

namespace Plunksna {

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct Context
{
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    QueueFamilyIndices familyIndices;
};

}

#endif //CONTEXT_H
