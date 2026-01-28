//
// Created by d on 1/28/26.
//

#include "SwapChain.h"
#include "Exception.h"
#include "RendererIncludes.h"

namespace Plunksna {

void SwapChain::init(VkDevice device, VkPhysicalDevice physicalDevice, const Window& window, QueueFamilyIndices families)
{
    m_familyIndices = families;
    m_device = device;
    m_physicalDevice = physicalDevice;

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, window);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    uint32_t queueFamilyIndices[] = {m_familyIndices.graphicsFamily.value(), m_familyIndices.presentFamily.value()};

    if (m_familyIndices.graphicsFamily != m_familyIndices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
        THROW("Could not create swapchain")
    }

    vkGetSwapchainImagesKHR(device, m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, m_swapChain, &imageCount, m_swapChainImages.data());

    m_swapChainExtent = extent;
    m_swapChainImageFormat = surfaceFormat.format;
}

void SwapChain::clean()
{
    vkDeviceWaitIdle(m_device);

    for (auto& buf : m_swapChainFramebuffers) {
        VK_DESTROY_F(buf, m_device, vkDestroyFramebuffer, nullptr)
    }

    for (auto& view : m_swapChainImageViews) {
        VK_DESTROY_F(view, m_device, vkDestroyImageView, nullptr)
    }

    VK_DESTROY(m_depthImage, m_device, vkDestroyImage)
    VK_DESTROY(m_depthImageView, m_device, vkDestroyImageView)
    VK_DESTROY(m_depthImageMemory, m_device, vkFreeMemory)

    VK_DESTROY_F(m_swapChain, m_device, vkDestroySwapchainKHR, nullptr)
}

void SwapChain::createImageViews()
{
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++)
        m_swapChainImageViews[i] = createImageView(m_device, m_swapChainImages[i], m_swapChainImageFormat);
}

void SwapChain::createDepthBuffers()
{
    VkFormat depthFormat = findDepthFormat(m_physicalDevice);
    // createImage(m_swapChainExtent.width, m_swapChainExtent.height, 1, depthFormat,
    //             VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    //             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    //             m_depthImage, m_depthImageMemory);

    m_depthImageView = createImageView(m_device, m_depthImage, depthFormat, 1, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void SwapChain::createFrameBuffers() {}

void SwapChain::regenerate(const Window& window, QueueFamilyIndices families)
{
    clean();

    init(m_device, m_physicalDevice, window, families);
    createImageViews();
    createDepthBuffers();
    createFrameBuffers();
}

SwapChainSupportDetails SwapChain::querySwapChainSupport(VkPhysicalDevice device, const Window& window)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace ==
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
            }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    if (m_forceVerticalSync)
        return VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window& window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int width, height;
    SDL_GetWindowSizeInPixels(window.getWindow(), &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                    capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                     capabilities.maxImageExtent.height);

    return actualExtent;
}
} // Plunksna