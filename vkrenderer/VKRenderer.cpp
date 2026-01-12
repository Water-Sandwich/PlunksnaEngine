//
// Created by d on 1/5/26.
//

#include "VKRenderer.h"

#include <cstring>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <set>

#include "Exception.h"
#include "Log.h"
#include "Window.h"

namespace Plunksna {
constexpr Severity vkToPkSev(VkDebugUtilsMessageSeverityFlagBitsEXT vkSev)
{
    switch (vkSev) {
    default:
        return Severity::eINFO;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        return Severity::eLETHAL;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        return Severity::eWARNING;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        return Severity::eVERBOSE;
    }
}

std::vector<VkLayerProperties> VKRenderer::getLayers()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    // LOG("Available VK layers:");
    // for (const auto& layer : layers) {
    //     LOG_C(layer.layerName);
    // }

    return layers;
}

std::vector<VkExtensionProperties> VKRenderer::getExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    // LOG("Available VK extensions:")
    // for (auto ext : extensions) {
    //     LOG_C(ext.extensionName);
    // }

    return extensions;
}

std::vector<const char*> VKRenderer::getRequiredExtensions()
{
    uint32_t sdlExtensionCount = 0;
    auto sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

    std::vector<const char*> extensions(sdlExtensions, sdlExtensions + sdlExtensionCount);

    if (s_enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool VKRenderer::checkValidationLayers()
{
    auto layers = getLayers();

    for (auto validLayer : s_validationLayers) {
        bool found = false;

        for (auto layer : layers) {
            if (strcmp(layer.layerName, validLayer) == 0) {
                found = true;
                break;
            }
        }

        if (!found)
            return false;
    }

    return true;
}


VKRenderer::VKRenderer()
{
    //init();
}

VKRenderer::~VKRenderer()
{
    clean();
}

VkInstance VKRenderer::createInstance()
{
    if (s_enableValidationLayers && !checkValidationLayers())
        THROW("Could not find validation layers")

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Plunksna";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    auto extensions = getRequiredExtensions();

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (s_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(s_validationLayers.size());
        createInfo.ppEnabledLayerNames = s_validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    createInfo.enabledExtensionCount = static_cast<Uint32>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    auto result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    if (result != VK_SUCCESS) {
        THROW(string_VkResult(result))
    }

    getPhysicalDevices();

    return m_instance;
}

VkInstance VKRenderer::init(const Window& window)
{
    if (m_instance == VK_NULL_HANDLE)
        createInstance();

    if (s_enableValidationLayers)
        initDebugger();

    selectDevice(window);
    createLogicalDevice(window);

    return m_instance;
}

void VKRenderer::clean()
{
    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }

    if (s_enableValidationLayers && m_debugger != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugger, nullptr);
        m_debugger = VK_NULL_HANDLE;
    }

    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

void VKRenderer::selectDevice(const Window& window)
{
    auto devices = getPhysicalDevices();

    for (const auto& device : devices) {
        if (isDeviceSuitable(device, window)) {
            m_physicalDevice = device;
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE)
        THROW("Failed to find suitable GPU")
}

std::vector<VkPhysicalDevice> VKRenderer::getPhysicalDevices()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if (deviceCount == 0)
        THROW("No devices with Vulkan support found")

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    return devices;
}

QueueFamilyIndices VKRenderer::findQueueFamilies(VkPhysicalDevice device, const Window& window)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, window.getSurface(), &presentSupport);
        if (presentSupport)
            indices.presentFamily = i;

        if (indices.isComplete())
            return indices;

        i++;
    }

    return indices;
}

bool VKRenderer::isDeviceSuitable(VkPhysicalDevice device, const Window& window)
{
    auto indices = findQueueFamilies(device, window);

    return indices.isComplete();
}

void VKRenderer::createLogicalDevice(const Window& window)
{
    auto indices = findQueueFamilies(m_physicalDevice, window);



    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &m_queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = 0;

    if (s_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(s_validationLayers.size());
        createInfo.ppEnabledLayerNames = s_validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
        THROW("Could not create logical device")

    vkGetDeviceQueue(m_device, indices.graphicsFamily.value() , 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);


}
} // Plunksna
