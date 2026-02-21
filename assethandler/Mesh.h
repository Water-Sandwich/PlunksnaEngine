//
// Created by d on 2/4/26.
//

#ifndef MESH_H
#define MESH_H

#include <cstdint>
#include <vector>

#include "Asset.h"
#include "vkrenderer/Buffer.h"
#include "vkrenderer/Vertex.h"

namespace Plunksna {

struct Mesh
{
    //host
    std::vector<Vertex> vertices;
    std::vector<u32> indices;

    VkDeviceSize verticesSize;
    VkDeviceSize indicesSize;

    u32 verticesCount;
    u32 indicesCount;

    //device
    // Buffer vertexBuffer;
    // Buffer indexBuffer;

    Buffer combinedBuffer;

    bool isHostLoaded() const
    {
        return !vertices.empty() && !indices.empty();
    }

    bool isDeviceLoaded() const
    {
        return combinedBuffer.buffer != VK_NULL_HANDLE;
    }
};

}

#endif //MESH_H
