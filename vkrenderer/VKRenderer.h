//
// Created by d on 1/5/26.
//

#ifndef VKRENDERER_H
#define VKRENDERER_H
#include <optional>
#include <vector>
#include <SDL3/SDL_stdinc.h>
#include <vulkan/vulkan_core.h>

#include "Window.h"

namespace Plunksna {

#ifdef NDEBUG
static const bool s_enableValidationLayers = false;
#else
static const bool s_enableValidationLayers = true;
#endif

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VKRenderer {
public:
    //=========BASE========
    VKRenderer();
    ~VKRenderer();

    VKRenderer(const VKRenderer&) = delete;
    VKRenderer(VKRenderer&&) = delete;

    VkInstance init(const Window& window);
    void clean();

    void selectDevice(const Window& window);


    //======LAYERS AND EXTENSIONS========

private:
    inline static const std::vector<const char*> s_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    inline static const std::vector<const char*> s_deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    void createInstance();

    std::vector<VkLayerProperties> getLayers();
    bool checkValidationLayers();

    std::vector<VkExtensionProperties> getExtensions();
    std::vector<const char*> getRequiredExtensions();

    std::vector<VkPhysicalDevice> getPhysicalDevices();
    bool isDeviceSuitable(VkPhysicalDevice device, const Window& window);

    void createLogicalDevice(const Window& window);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, const Window& window);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, const Window& window);


    //=====SWAPCHAIN=====

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window& window);

    void createSwapChain(const Window& window);
    void createSurface(const Window& window);

    void createImageViews();
    void createGraphicsPipeline();

    static std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    void createRenderPass();

public:
    VkDebugUtilsMessengerEXT m_debugger = VK_NULL_HANDLE;
    VkInstance m_instance = VK_NULL_HANDLE;

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;

    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;

    const float m_queuePriority = 1.f;
    const bool m_forceVSync = true;

private:
    //=======DEBUG=========

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    void initDebugger();
};

} // Plunksna

#endif //VKRENDERER_H
