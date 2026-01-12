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

class VKRenderer {
public:
    //=========BASE========
    VKRenderer();
    ~VKRenderer();

    VKRenderer(const VKRenderer&) = delete;
    VKRenderer(VKRenderer&&) = delete;

    VkInstance createInstance();
    VkInstance init(const Window& window);
    void clean();

    void selectDevice(const Window& window);


    //======LAYERS AND EXTENSIONS========

private:
    inline static const std::vector<const char*> s_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };


    std::vector<VkLayerProperties> getLayers();
    bool checkValidationLayers();

    std::vector<VkExtensionProperties> getExtensions();
    std::vector<const char*> getRequiredExtensions();

    std::vector<VkPhysicalDevice> getPhysicalDevices();
    bool isDeviceSuitable(VkPhysicalDevice device, const Window& window);

    void createLogicalDevice(const Window& window);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, const Window& window);

public:
    VkDebugUtilsMessengerEXT m_debugger = VK_NULL_HANDLE;
    VkInstance m_instance = VK_NULL_HANDLE;

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    const float m_queuePriority = 1.f;

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
