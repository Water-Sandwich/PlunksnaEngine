//
// Created by d on 1/28/26.
//

#ifndef FRAMERESOURCE_H
#define FRAMERESOURCE_H

#include <vector>
#include <vulkan/vulkan_core.h>
#include "Context.h"
#include "RendererUtils.h"

namespace Plunksna {

struct FrameResource {
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    VkBuffer uniformBuffer = VK_NULL_HANDLE;
    void* uniformBufferMapped = nullptr;
    VkDeviceMemory uniformBufferMemory = VK_NULL_HANDLE;

    VkSemaphore imageAvailableSem = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSem = VK_NULL_HANDLE;
    VkFence frameInFlightFence = VK_NULL_HANDLE;

    void destroyBuffers(const Context& context)
    {
        VK_DESTROY(uniformBuffer, context.device, vkDestroyBuffer)
        VK_DESTROY(uniformBufferMemory, context.device, vkFreeMemory)
    }

    void destroySync(const Context& context)
    {
        VK_DESTROY(imageAvailableSem, context.device, vkDestroySemaphore)
        VK_DESTROY(renderFinishedSem, context.device, vkDestroySemaphore)
        VK_DESTROY(frameInFlightFence, context.device, vkDestroyFence)
    }
};

} // Plunksna

#endif //FRAMERESOURCE_H
