//
// Created by d on 1/28/26.
//

#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <vector>
#include <vulkan/vulkan_core.h>

#include "Context.h"
#include "FrameResource.h"
#include "RendererUtils.h"
#include "Window.h"

namespace Plunksna {

class SwapChain {
public:
    explicit SwapChain(Context& context);
    ~SwapChain() = default;

    SwapChain() = delete;
    SwapChain(const SwapChain&) = delete;
    SwapChain(SwapChain&&) = delete;

    void createSurface(const Window& window);
    void init(const Window& window, bool vSync);
    void initResources();

    void regenerate(const Window& window, bool vSync);
    void clean();
    void cleanSurface();

    //draw
    VkResult fetch(const FrameResource& resource, uint32_t& imageIndex) const;
    VkResult present(VkQueue presentQueue, uint32_t imageIndex);

    VkExtent2D extent() const;
    int width() const;
    int height() const;
    VkFormat format() const;

    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device) const;
    RenderUtils::SwapChainSupportDetails getSupport(VkPhysicalDevice device) const;
    VkFramebuffer getFrameBuffer(uint32_t index) const;
    VkSemaphore getRenderFinishedSemaphore(uint32_t index) const;
private:
    void createImageViews();
    void createDepthBuffers();
    void createFrameBuffers();
    void createSampledImage();
    void createSemaphores();

private:
    Context& m_context;

    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkFormat m_imageFormat;
    VkExtent2D m_extent;

    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    std::vector<VkFramebuffer> m_framebuffers;
    std::vector<VkSemaphore> m_renderFinished;

    VkImage m_depthImage = VK_NULL_HANDLE;
    VkImageView m_depthImageView = VK_NULL_HANDLE;
    VkDeviceMemory m_depthImageMemory = VK_NULL_HANDLE;

    VkImage m_colorImage = VK_NULL_HANDLE;
    VkImageView m_colorImageView = VK_NULL_HANDLE;
    VkDeviceMemory m_colorImageMemory = VK_NULL_HANDLE;
};

} // Plunksna

#endif //SWAPCHAIN_H