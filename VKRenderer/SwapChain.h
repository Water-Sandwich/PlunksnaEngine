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
#include "Engine/Window.h"
#include "Image.h"
#include "Utils/Types.h"

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
    VkResult fetch(const FrameResource& resource, u32& imageIndex) const;
    VkResult present(VkQueue presentQueue, u32 imageIndex);

    VkExtent2D extent() const;
    i32 width() const;
    i32 height() const;
    VkFormat format() const;

    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device) const;
    RenderUtils::SwapChainSupportDetails getSupport(VkPhysicalDevice device) const;
    VkFramebuffer getFrameBuffer(u32 index) const;
    VkSemaphore getRenderFinishedSemaphore(u32 index) const;

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

    Image m_depthImage;
    VkImageView m_depthImageView = VK_NULL_HANDLE;

    Image m_colorImage;
    VkImageView m_colorImageView = VK_NULL_HANDLE;
};

} // Plunksna

#endif //SWAPCHAIN_H