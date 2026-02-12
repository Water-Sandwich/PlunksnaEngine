//
// Created by d on 1/28/26.
//

#ifndef FRAMERESOURCE_H
#define FRAMERESOURCE_H

#include <vector>
#include <vulkan/vulkan_core.h>

#include "Buffer.h"
#include "Context.h"
#include "RendererUtils.h"

namespace Plunksna {

struct FrameResource {
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    void* uniformBufferMapped = nullptr;
    Buffer uniformBuffer;

    VkSemaphore imageAvailableSem = VK_NULL_HANDLE;
    VkFence frameInFlightFence = VK_NULL_HANDLE;

    void destroyBuffers(const Context& context)
    {
        vmaUnmapMemory(context.allocator, uniformBuffer.allocation);
        uniformBuffer.destroy(context);
    }

    void destroySync(const Context& context)
    {
        VK_DESTROY(imageAvailableSem, context.device, vkDestroySemaphore)
        VK_DESTROY(frameInFlightFence, context.device, vkDestroyFence)
    }
};

} // Plunksna

#endif //FRAMERESOURCE_H
