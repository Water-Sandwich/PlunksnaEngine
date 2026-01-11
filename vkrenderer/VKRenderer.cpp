//
// Created by d on 1/5/26.
//

#include "VKRenderer.h"

#include "Log.h"

namespace Plunksna {
std::vector<VkLayerProperties> VKRenderer::getLayers()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    LOG("Available VK layers:");
    for (const auto& layer : layers) {
        LOG_C(layer.layerName);
    }

    return layers;
}

std::vector<VkExtensionProperties> VKRenderer::getExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    LOG("Available VK extensions:")
    for (auto ext : extensions) {
        LOG_C(ext.extensionName);
    }

    return extensions;
}
} // Plunksna