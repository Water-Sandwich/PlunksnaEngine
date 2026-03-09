//
// Created by d on 2/13/26.
//

#ifndef ASSET_H
#define ASSET_H

#include <limits>
#include "Utils/Types.h"

namespace Plunksna {

using Asset = u32;
constexpr Asset NULL_ASSET = std::numeric_limits<Asset>::max();

}

#endif //ASSET_H
