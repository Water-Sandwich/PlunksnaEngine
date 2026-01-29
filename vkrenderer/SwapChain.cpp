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

void SwapChain::init(const Window& window, bool vSync)
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_context.physicalDevice, m_surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, vSync);
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

    if (vkCreateSwapchainKHR(m_context.device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
        THROW("Could not create swapchain")
    }

    vkGetSwapchainImagesKHR(m_context.device, m_swapChain, &imageCount, nullptr);
    m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_context.device, m_swapChain, &imageCount, m_images.data());

    m_extent = extent;
    m_imageFormat = surfaceFormat.format;
}

void SwapChain::initResources()
{
    createImageViews();
    createDepthBuffers();
    createSampledImage();
    createFrameBuffers();
}

QueueFamilyIndices SwapChain::getQueueFamilies(VkPhysicalDevice device) const
{
    return RenderUtils::findQueueFamilies(device, m_surface);
}

SwapChainSupportDetails SwapChain::getSupport(VkPhysicalDevice device) const
{
    return querySwapChainSupport(device, m_surface);
}

VkFramebuffer SwapChain::getFrameBuffer(uint32_t index) const
{
    return m_framebuffers[index];
}

void SwapChain::clean()
{
    vkDeviceWaitIdle(m_context.device);

    for (auto& buf : m_framebuffers)
        VK_DESTROY_F(buf, m_context.device, vkDestroyFramebuffer, nullptr)

    for (auto& view : m_imageViews)
        VK_DESTROY_F(view, m_context.device, vkDestroyImageView, nullptr)

    VK_DESTROY(m_depthImage, m_context.device, vkDestroyImage)
    VK_DESTROY(m_depthImageView, m_context.device, vkDestroyImageView)
    VK_DESTROY(m_depthImageMemory, m_context.device, vkFreeMemory)

    VK_DESTROY(m_colorImage, m_context.device, vkDestroyImage)
    VK_DESTROY(m_colorImageView, m_context.device, vkDestroyImageView)
    VK_DESTROY(m_colorImageMemory, m_context.device, vkFreeMemory)

    VK_DESTROY_F(m_swapChain, m_context.device, vkDestroySwapchainKHR, nullptr)
}

void SwapChain::cleanSurface()
{
    VK_DESTROY(m_surface, m_context.instance, vkDestroySurfaceKHR)
}

VkResult SwapChain::fetch(const FrameResource& resource, uint32_t& imageIndex) const
{
    return vkAcquireNextImageKHR(
        m_context.device, m_swapChain, UINT64_MAX,
        resource.imageAvailableSem,
        VK_NULL_HANDLE, &imageIndex);
}

VkResult SwapChain::present(const FrameResource& resource, VkQueue presentQueue, uint32_t imageIndex)
{
    VkSemaphore signalSemaphores[] = {resource.renderFinishedSem};

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    return vkQueuePresentKHR(presentQueue, &presentInfo);
}

VkExtent2D SwapChain::extent() const
{
    return m_extent;
}

int SwapChain::width() const
{
    return m_extent.width;
}

int SwapChain::height() const
{
    return m_extent.height;
}

VkFormat SwapChain::format() const
{
    return m_imageFormat;
}

void SwapChain::createImageViews()
{
    m_imageViews.resize(m_images.size());

    for (size_t i = 0; i < m_images.size(); i++)
        m_imageViews[i] = createImageView(m_context, m_images[i], m_imageFormat);
}

void SwapChain::createDepthBuffers()
{
    VkFormat depthFormat = findDepthFormat(m_context);

    //TODO: dont msaa depth buffer?
    createImage(m_context, m_extent.width, m_extent.height, 1, depthFormat,
                       VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                       m_depthImage, m_depthImageMemory, m_context.msaaSamples);

    m_depthImageView = createImageView(m_context, m_depthImage, depthFormat, 1, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void SwapChain::createFrameBuffers()
{
    m_framebuffers.resize(m_imageViews.size());

    for (size_t i = 0; i < m_imageViews.size(); i++) {
        std::array<VkImageView, 3> attachments = {
            m_colorImageView,
            m_depthImageView,
            m_imageViews[i],
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_context.renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_extent.width;
        framebufferInfo.height = m_extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_context.device, &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS) {
            THROW("failed to create framebuffer!");
        }
    }
}

void SwapChain::createSampledImage()
{
    createImage(m_context, m_extent.width, m_extent.height, 1, m_imageFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_colorImage, m_colorImageMemory, m_context.msaaSamples);

    m_colorImageView = createImageView(m_context, m_colorImage, m_imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void SwapChain::regenerate(const Window& window, bool vSync)
{
    clean();

    init(window, vSync);
    initResources();
}

} // Plunksna