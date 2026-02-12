//
// Created by d on 1/28/26.
//

#ifndef RENDERERINCLUDES_H
#define RENDERERINCLUDES_H

#include <vulkan/vulkan_core.h>
#include <vector>
#include <string>
#include "Context.h"
#include "Log.h"
#include "Window.h"
#include "Image.h"

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
VkImageView createImageView(const Context& context, VkImage image, VkFormat format, uint32_t mipLevels = 1,
                            VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);

VkFormat findDepthFormat(const Context& context);

VkFormat findSupportedFormat(const Context& context, const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                            VkFormatFeatureFlags features);

//[[deprecated]]
void createImage(const Context& context, uint32_t width, uint32_t height, uint32_t mipLevels,
                VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                VkImage& image, VkDeviceMemory& imageMemory, VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_1_BIT);

void createImage(const Context& context, Image& image, uint32_t width, uint32_t height, uint32_t mipLevels,
                VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                VkSampleCountFlagBits numSamples, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);


//extensions and layers
std::vector<VkLayerProperties> getLayers();

std::vector<VkExtensionProperties> getExtensions();

std::vector<const char*> getRequiredExtensions(bool isDebug);

bool checkValidationLayers(const std::vector<const char*>& requiredValidation);

bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& extensions);


//capabilities
bool hasStencil(VkFormat format);

float getMaxAnisotropy(const Context& context);

uint32_t findMemoryType(const Context& context, uint32_t typeFilter, VkMemoryPropertyFlags properties);

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

std::vector<VkPhysicalDevice> getPhysicalDevices(const Context& context);

VkSampleCountFlagBits getMaxMSAA(VkPhysicalDevice device);

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
