//
// Created by d on 1/28/26.
//

#ifndef QUEUEFAMILYINDICES_H
#define QUEUEFAMILYINDICES_H

#include <optional>

namespace Plunksna {

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

}

#endif //QUEUEFAMILYINDICES_H
