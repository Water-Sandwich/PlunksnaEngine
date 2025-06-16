//
// Created by d on 5/17/25.
//
#ifndef PAGINATEDVECTOR_TPP
#define PAGINATEDVECTOR_TPP

#include "../engine/Log.h"
#include "PaginatedVector.h"

namespace Plunksna {
template <typename T, T defaultValue, std::size_t pageSize>
constexpr T& PaginatedVector<T, defaultValue, pageSize>::operator[](std::size_t index)
{
    auto& page = pages[index / pageSize];
    return page[index % pageSize];
}

template <typename T, T defaultValue, std::size_t pageSize>
constexpr T PaginatedVector<T, defaultValue, pageSize>::operator[](std::size_t index) const
{
    auto& page = pages[index / pageSize];
    return page[index % pageSize];
}

template <typename T, T defaultValue, std::size_t pageSize>
template <typename ... Args>
constexpr void PaginatedVector<T, defaultValue, pageSize>::emplace(std::size_t index, Args&&... args)
{
    auto& page = getOrCreatePage(index);
    page[index % pageSize] = T(std::forward<Args>(args)...);
}

template <typename T, T defaultValue, std::size_t pageSize>
constexpr void PaginatedVector<T, defaultValue, pageSize>::insert(std::size_t index, const T& value)
{
    auto& page = getOrCreatePage(index);
    page[index % pageSize] = value;
}

template <typename T, T defaultValue, std::size_t pageSize>
constexpr std::vector<T>& PaginatedVector<T, defaultValue, pageSize>::getOrCreatePage(std::size_t index)
{
    auto pageIndex = index / pageSize;
    if (pageIndex < pages.size() && pages[pageIndex].size() != 0)
        return pages[pageIndex];

    if (pageIndex >= pages.size())
        pages.resize(pageIndex + 1);

    //constexpr bool hasDefault = defaultValue == T{};
    return initPage(pages[pageIndex]);
}

template <typename T, T defaultValue, std::size_t pageSize>
constexpr std::vector<T>& PaginatedVector<T, defaultValue, pageSize>::initPage(std::vector<T>& page)
{
    return page = std::vector<T>(pageSize, defaultValue);
}

} // Plunksna
#endif // PAGINATEDVECTOR_TPP
