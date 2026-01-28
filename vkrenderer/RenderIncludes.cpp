//
// Created by d on 1/28/26.
//

#include "Exception.h"
#include "RendererIncludes.h"

namespace Plunksna {

constexpr Severity vkToPkSev(VkDebugUtilsMessageSeverityFlagBitsEXT vkSev)
{
    switch (vkSev) {
    default:
        return Severity::eINFO;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        return Severity::eLETHAL;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        return Severity::eWARNING;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        return Severity::eVERBOSE;
    }
}

VkImageView createImageView(const Context& context, VkImage image, VkFormat format, uint32_t mipLevels,
                                   VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;

    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;

    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    createInfo.subresourceRange.levelCount = mipLevels;

    VkImageView view;

    if (vkCreateImageView(context.device, &createInfo, nullptr, &view) != VK_SUCCESS) {
        THROW("Could not create image view")
    }

    return view;
}

VkFormat findSupportedFormat(const Context& context, const std::vector<VkFormat>& candidates,
    VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(context.physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    THROW("Couldnt find a supported format")
}

VkFormat findDepthFormat(const Context& context)
{
    return findSupportedFormat(context,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

}