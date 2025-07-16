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

constexpr unsigned char MAX_COMPONENTS = 32;
using ComponentMask = std::bitset<MAX_COMPONENTS>;

constexpr static std::size_t FILTER_RESERVE_SIZE = 1024;

}

#endif //ENTITY_H
