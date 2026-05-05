//
// Created by d on 5/5/26.
//

#include "SparseSet.h"

namespace Plunksna {

template <typename T, std::size_t PageSize>
constexpr T& SparseSet<T, PageSize>::operator[](std::size_t index)
{
    return m_members[m_indexes[index]];
}

template <typename T, std::size_t PageSize>
constexpr T SparseSet<T, PageSize>::operator[](std::size_t index) const
{
    return m_members[m_indexes[index]];
}

template <typename T, std::size_t PageSize>
constexpr void SparseSet<T, PageSize>::push(const T& value)
{

}
} // Plunksna