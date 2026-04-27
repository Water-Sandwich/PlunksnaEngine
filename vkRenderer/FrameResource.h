//
// Created by d on 1/28/26.
//

#ifndef FRAMERESOURCE_H
#define FRAMERESOURCE_H

#include <vector>
#include <vulkan/vulkan_core.h>
#include <tracy/TracyVulkan.hpp>

#include "Buffer.h"
#include "Context.h"
#include "RendererUtils.h"

namespace Plunksna {

struct FrameResource {
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    //TODO: tie this to descriptor manager
    void* cameraMap = nullptr;
    Buffer cameraBuffer;

    void* objsMap = nullptr;
    Buffer objsBuffer;

    VkSemaphore imageAvailableSem = VK_NULL_HANDLE;
    VkFence frameInFlightFence = VK_NULL_HANDLE;

    void destroyBuffers(const Context& context)
    {
        vmaUnmapMemory(context.allocator, cameraBuffer.allocation);
        cameraBuffer.destroy(context);
        vmaUnmapMemory(context.allocator, objsBuffer.allocation);
        objsBuffer.destroy(context);
    }

    void destroySync(const Context& context)
    {
        VK_DESTROY(imageAvailableSem, context.device, vkDestroySemaphore)
        VK_DESTROY(frameInFlightFence, context.device, vkDestroyFence)
    }
};

} // Plunksna

#endif //FRAMERESOURCE_H
