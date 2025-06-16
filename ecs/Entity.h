//
// Created by d on 6/12/25.
//

#ifndef ENTITY_H
#define ENTITY_H

#include <bitset>
#include <limits>

namespace Plunksna {

using Entity = unsigned long int;
constexpr Entity NULL_ENTITY = std::numeric_limits<Entity>::max();
constexpr Entity NULL_INDEX = std::numeric_limits<std::size_t>::max();

constexpr unsigned short MAX_COMPONENTS = 32;
using ComponentMask = std::bitset<MAX_COMPONENTS>;

}

#endif //ENTITY_H
