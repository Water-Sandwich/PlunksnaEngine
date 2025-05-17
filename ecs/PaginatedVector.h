//
// Created by d on 5/17/25.
//

#ifndef PAGINATEDVECTOR_H
#define PAGINATEDVECTOR_H
#include <optional>
#include <vector>

namespace Plunksna {

template <
    typename T,
    T defaultValue = T{},
    std::size_t pageSize = 512
>
class PaginatedVector {
public:
    constexpr T& operator[](std::size_t index);

    constexpr T operator[](std::size_t index) const;

    template <typename... Args>
    constexpr void emplace(std::size_t index, Args&&... args);

    constexpr void insert(std::size_t index, const T& value);

private:
    constexpr std::vector<T>& getOrCreatePage(std::size_t index);

    constexpr std::vector<T>& initPage(std::vector<T>& page);

private:
    std::vector<std::vector<T>> pages;
};

} // Plunksna

#include "PaginatedVector.tpp"

#endif //PAGINATEDVECTOR_H
