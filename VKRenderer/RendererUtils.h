//
// Created by d on 1/28/26.
//

#ifndef RENDERERINCLUDES_H
#define RENDERERINCLUDES_H

#include <vulkan/vulkan_core.h>
#include <vector>
#include <string>
#include "Context.h"
#include "Engine/Log.h"
#include "Engine/Window.h"
#include "Image.h"
#include "Utils/Types.h"

namespace Plunksna::RenderUtils {

#define VK_DESTROY_F(obj, parent, func, ...) \
    if(obj != VK_NULL_HANDLE){func(parent, obj, __VA_ARGS__);obj = VK_NULL_HANDLE;}

#define VK_DESTROY(obj, parent, func) VK_DESTROY_F(obj, parent, func, nullptr)

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


//==========================
//==========HELPER FUNCTIONS
//==========================


constexpr Severity vkToPkSev(VkDebugUtilsMessageSeverityFlagBitsEXT vkSev);

//images
VkImageView createImageView(const Context& context, VkImage image, VkFormat format, u32 mipLevels = 1,
                            VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);

VkFormat findDepthFormat(const Context& context);

VkFormat findSupportedFormat(const Context& context, const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                            VkFormatFeatureFlags features);

void createImage(const Context& context, Image& image, u32 width, u32 height, u32 mipLevels,
                VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_1_BIT);

u32 getMipLevels(u32 width, u32 height);

//extensions and layers
std::vector<VkLayerProperties> getLayers();

std::vector<VkExtensionProperties> getExtensions();

std::vector<const char*> getRequiredExtensions(bool isDebug);

bool checkValidationLayers(const std::vector<const char*>& requiredValidation);

bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& extensions);


//capabilities
bool hasStencil(VkFormat format);

f32 getMaxAnisotropy(const Context& context);

u32 findMemoryType(const Context& context, u32 typeFilter, VkMemoryPropertyFlags properties);

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

std::vector<VkPhysicalDevice> getPhysicalDevices(const Context& context);

VkSampleCountFlagBits getMaxMSAA(VkPhysicalDevice device);

VkDeviceSize alignedSize(VkDeviceSize size, VkDeviceSize alignment);

//swapchain
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window& window);

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vSync);

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);


//assets
//TODO: Make asset handler
std::vector<char> readFile(const std::string& filename);

VkShaderModule createShaderModule(const Context& context, const std::vector<char>& code);

}

#endif //RENDERERINCLUDES_H
