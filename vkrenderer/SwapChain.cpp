//
// Created by d on 1/28/26.
//

#include "SwapChain.h"
#include "Exception.h"
#include "RendererUtils.h"

namespace Plunksna {

using namespace Plunksna::RenderUtils;

SwapChain::SwapChain(Context& context) : m_context(context) {}

void SwapChain::createSurface(const Window& window)
{
    if (!SDL_Vulkan_CreateSurface(window.getWindow(), m_context.instance, nullptr, &m_surface))
        THROW("Could not create surface")
}

void SwapChain::init(const Context& context, const Window& window)
{
    m_context = context;

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(context.physicalDevice, m_surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, m_forceVerticalSync);
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

    uint32_t queueFamilyIndices[] = {m_context.familyIndices.graphicsFamily.value(), m_context.familyIndices.presentFamily.value()};

    if (m_context.familyIndices.graphicsFamily != m_context.familyIndices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    if (vkCreateSwapchainKHR(context.device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
        THROW("Could not create swapchain")
    }

    vkGetSwapchainImagesKHR(context.device, m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(context.device, m_swapChain, &imageCount, m_swapChainImages.data());

    m_swapChainExtent = extent;
    m_swapChainImageFormat = surfaceFormat.format;
}

void SwapChain::clean()
{
    vkDeviceWaitIdle(m_context.device);

    for (auto& buf : m_swapChainFramebuffers)
        VK_DESTROY_F(buf, m_context.device, vkDestroyFramebuffer, nullptr)

    for (auto& view : m_swapChainImageViews)
        VK_DESTROY_F(view, m_context.device, vkDestroyImageView, nullptr)

    VK_DESTROY(m_depthImage, m_context.device, vkDestroyImage)
    VK_DESTROY(m_depthImageView, m_context.device, vkDestroyImageView)
    VK_DESTROY(m_depthImageMemory, m_context.device, vkFreeMemory)

    VK_DESTROY_F(m_swapChain, m_context.device, vkDestroySwapchainKHR, nullptr)
}

void SwapChain::createImageViews()
{
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++)
        m_swapChainImageViews[i] = createImageView(m_context, m_swapChainImages[i], m_swapChainImageFormat);
}

void SwapChain::createDepthBuffers()
{
    VkFormat depthFormat = findDepthFormat(m_context);

    createImage(m_context, m_swapChainExtent.width, m_swapChainExtent.height, 1, depthFormat,
                       VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                       m_depthImage, m_depthImageMemory);

    m_depthImageView = createImageView(m_context, m_depthImage, depthFormat, 1, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void SwapChain::createFrameBuffers()
{
    m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

    for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            m_swapChainImageViews[i],
            m_depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_context.renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_swapChainExtent.width;
        framebufferInfo.height = m_swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_context.device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
            THROW("failed to create framebuffer!");
        }
    }
}

void SwapChain::regenerate(const Window& window)
{
    clean();

    init(m_context, window);
    createImageViews();
    createDepthBuffers();
    createFrameBuffers();
}

} // Plunksna