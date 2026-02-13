//
// Created by d on 2/4/26.
//

#ifndef MESH_H
#define MESH_H

#include <cstdint>
#include <vector>

#include "Asset.h"
#include "Buffer.h"
#include "Vertex.h"

namespace Plunksna {

struct Mesh
{
    Asset handle = NULL_ASSET;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    Buffer vertexBuffer;
    Buffer indexBuffer;
};

}

#endif //MESH_H
