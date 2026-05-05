//
// Created by d on 5/5/26.
//

#ifndef SPARSESET_H
#define SPARSESET_H
#include <cstddef>

#include "Entity.h"
#include "PaginatedVector.h"

namespace Plunksna {

//keep members in a packed vector, with indexes to the packed vector being sparsely held
template <typename T, std::size_t PageSize = 512>
class SparseSet {
public:
    SparseSet() = default;

    constexpr T& operator[](std::size_t index);
    constexpr T operator[](std::size_t index) const;

    constexpr void push(const T& value);

private:
    PaginatedVector<std::size_t, NULL_INDEX, PageSize> m_indexes; // Sparse
    std::vector<T> m_members; // Dense
};


#include "SparseSet.tpp"

} // Plunksna

#endif //SPARSESET_H
