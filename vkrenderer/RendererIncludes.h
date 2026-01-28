//
// Created by d on 1/28/26.
//

#ifndef RENDERERINCLUDES_H
#define RENDERERINCLUDES_H

#include <vulkan/vulkan_core.h>
#include <vector>
#include "Context.h"

namespace Plunksna {

#define VK_DESTROY_F(obj, parent, func, ...) \
if(obj != VK_NULL_HANDLE){func(parent, obj, __VA_ARGS__);obj = VK_NULL_HANDLE;}

#define VK_DESTROY(obj, parent, func) VK_DESTROY_F(obj, parent, func, nullptr)

constexpr Severity vkToPkSev(VkDebugUtilsMessageSeverityFlagBitsEXT vkSev);

VkImageView createImageView(const Context& context, VkImage image, VkFormat format, uint32_t mipLevels = 1,
                                   VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);

VkFormat findSupportedFormat(const Context& context, const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                       VkFormatFeatureFlags features);

VkFormat findDepthFormat(const Context& context);

void createImage(const Context& context, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling,
                 VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

uint32_t findMemoryType(const Context& context, uint32_t typeFilter, VkMemoryPropertyFlags properties);
}

#endif //RENDERERINCLUDES_H
