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
    std::vector<uint32_t> indices;

    uint32_t verticesSize;
    uint32_t indicesSize;

    //device
    Buffer vertexBuffer;
    Buffer indexBuffer;

    bool isHostLoaded() const
    {
        return !vertices.empty() && !indices.empty();
    }

    bool isDeviceLoaded() const
    {
        return vertexBuffer.buffer != VK_NULL_HANDLE && indexBuffer.buffer != VK_NULL_HANDLE;
    }
};

}

#endif //MESH_H
