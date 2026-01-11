//
// Created by d on 1/5/26.
//

#ifndef VKRENDERER_H
#define VKRENDERER_H
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Plunksna {

class VKRenderer {
public:
    static std::vector<VkLayerProperties> getLayers();
    static std::vector<VkExtensionProperties> getExtensions();
};

} // Plunksna

#endif //VKRENDERER_H
