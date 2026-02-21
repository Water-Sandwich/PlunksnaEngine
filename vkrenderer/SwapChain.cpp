//
// Created by d on 1/28/26.
//

#include "SwapChain.h"
#include "engine/Exception.h"
#include "RendererUtils.h"

namespace Plunksna {

using namespace Plunksna::RenderUtils;

SwapChain::SwapChain(Context& context) : m_context(context) {}

void SwapChain::createSurface(const Window& window)
{
    ASSERT(SDL_Vulkan_CreateSurface(window.getWindow(), m_context.instance, nullptr, &m_surface),
        "Could not create surface")
}

void SwapChain::init(const Window& window, bool vSync)
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_context.physicalDevice, m_surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, vSync);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

    u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

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

    u32 queueFamilyIndices[] = {m_context.familyIndices.graphicsFamily.value(), m_context.familyIndices.presentFamily.value()};

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

    ASSERT_V(vkCreateSwapchainKHR(m_context.device, &createInfo, nullptr, &m_swapChain),
        "Could not create swapchain")

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
    createSemaphores();
}

QueueFamilyIndices SwapChain::getQueueFamilies(VkPhysicalDevice device) const
{
    return RenderUtils::findQueueFamilies(device, m_surface);
}

SwapChainSupportDetails SwapChain::getSupport(VkPhysicalDevice device) const
{
    return querySwapChainSupport(device, m_surface);
}

VkFramebuffer SwapChain::getFrameBuffer(u32 index) const
{
    return m_framebuffers[index];
}

VkSemaphore SwapChain::getRenderFinishedSemaphore(u32 index) const
{
    return m_renderFinished[index];
}

void SwapChain::clean()
{
    vkDeviceWaitIdle(m_context.device);

    for (auto& buf : m_framebuffers)
        VK_DESTROY(buf, m_context.device, vkDestroyFramebuffer)

    for (auto& view : m_imageViews)
        VK_DESTROY(view, m_context.device, vkDestroyImageView)

    for (auto& sem : m_renderFinished)
        VK_DESTROY(sem, m_context.device, vkDestroySemaphore)

    VK_DESTROY(m_depthImageView, m_context.device, vkDestroyImageView)
    m_depthImage.destroy(m_context);

    VK_DESTROY(m_colorImageView, m_context.device, vkDestroyImageView)
    m_colorImage.destroy(m_context);

    VK_DESTROY(m_swapChain, m_context.device, vkDestroySwapchainKHR)
}

void SwapChain::cleanSurface()
{
    VK_DESTROY(m_surface, m_context.instance, vkDestroySurfaceKHR)
}

VkResult SwapChain::fetch(const FrameResource& resource, u32& imageIndex) const
{
    return vkAcquireNextImageKHR(
        m_context.device, m_swapChain, UINT64_MAX,
        resource.imageAvailableSem,
        VK_NULL_HANDLE, &imageIndex);
}

VkResult SwapChain::present(VkQueue presentQueue, u32 imageIndex)
{
    VkSemaphore signalSemaphores[] = {m_renderFinished[imageIndex]};

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

i32 SwapChain::width() const
{
    return m_extent.width;
}

i32 SwapChain::height() const
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

    createImage(m_context, m_depthImage, m_extent.width, m_extent.height, 1, depthFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, m_context.msaaSamples);

    m_depthImageView = createImageView(m_context, m_depthImage.image, depthFormat, 1, VK_IMAGE_ASPECT_DEPTH_BIT);
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
        framebufferInfo.attachmentCount = static_cast<u32>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_extent.width;
        framebufferInfo.height = m_extent.height;
        framebufferInfo.layers = 1;

        ASSERT_V(vkCreateFramebuffer(m_context.device, &framebufferInfo, nullptr, &m_framebuffers[i]),
            "failed to create framebuffer!")
    }
}

void SwapChain::createSampledImage()
{
    createImage(m_context, m_colorImage, m_extent.width, m_extent.height, 1, m_imageFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, m_context.msaaSamples);

    m_colorImageView = createImageView(m_context, m_colorImage.image, m_imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void SwapChain::createSemaphores()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    m_renderFinished.resize(m_images.size());

    for (i32 i = 0; i < m_renderFinished.size(); i++) {
        ASSERT_V(vkCreateSemaphore(m_context.device, &semaphoreInfo, nullptr, &m_renderFinished[i]),
            "Could not create semaphore")
    }
}

void SwapChain::regenerate(const Window& window, bool vSync)
{
    clean();

    init(window, vSync);
    initResources();
}

} // Plunksna