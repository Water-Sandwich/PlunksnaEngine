//
// Created by d on 2/4/26.
//

#ifndef SHADERMODULE_H
#define SHADERMODULE_H

#include <vector>
#include <vulkan/vulkan_core.h>

namespace Plunksna {

struct ShaderModule
{
    //host
    std::vector<char> byteCode;

    //device
    VkShaderModule shaderModule = VK_NULL_HANDLE;

    bool isHostLoaded() const
    {
        return !byteCode.empty();
    }

    bool isDeviceLoaded() const
    {
        return shaderModule != VK_NULL_HANDLE;
    }
};

}

#endif //SHADERMODULE_H
