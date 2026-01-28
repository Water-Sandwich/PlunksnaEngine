//
// Created by d on 1/28/26.
//

#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <vector>
#include <vulkan/vulkan_core.h>

#include "Context.h"
#include "RendererUtils.h"
#include "Window.h"

namespace Plunksna {

class SwapChain {
public:
    SwapChain(Context& context);
    ~SwapChain() = default;

    SwapChain() = delete;
    SwapChain(const SwapChain&) = delete;
    SwapChain(SwapChain&&) = delete;

    void init(const Window& window);
    void createSurface(const Window& window);

    void regenerate(const Window& window);
    void clean();

    VkSwapchainKHR swapChain() const;
    VkSurfaceKHR surface() const;

private:

    void createImageViews();
    void createDepthBuffers();
    void createFrameBuffers();

    RenderUtils::SwapChainSupportDetails querySwapChainSupport(const Window& window);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window& window);

private:
    Context& m_context;

    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;

    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    VkImage m_depthImage = VK_NULL_HANDLE;
    VkImageView m_depthImageView = VK_NULL_HANDLE;
    VkDeviceMemory m_depthImageMemory = VK_NULL_HANDLE;

    bool m_forceVerticalSync = true;
};

} // Plunksna

#endif //SWAPCHAIN_H
