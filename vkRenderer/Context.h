//
// Created by d on 1/28/26.
//

#ifndef CONTEXT_H
#define CONTEXT_H

#include <optional>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include "utils/Types.h"

namespace Plunksna {

struct QueueFamilyIndices
{
    std::optional<u32> graphicsFamily;
    std::optional<u32> presentFamily;

    bool isComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct Context
{
    VkInstance instance = VK_NULL_HANDLE;
    VmaAllocator allocator;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkRenderPass imguiRenderPass = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    QueueFamilyIndices familyIndices;
    VkCommandPool m_transientCommandPool = VK_NULL_HANDLE;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkBool32 sampleShading = VK_FALSE;

    VkPhysicalDeviceProperties physicalDeviceProperties;
};

}

#endif //CONTEXT_H
