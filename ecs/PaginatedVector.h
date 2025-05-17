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
    constexpr T& operator[](std::size_t index)
    {
        auto& page = pages[index / pageSize];
        return page[index % pageSize];
    }

    constexpr T operator[](std::size_t index) const
    {
        auto& page = pages[index / pageSize];
        return page[index % pageSize];
    }

    template <typename... Args>
    constexpr void emplace(std::size_t index, Args&&... args)
    {
        auto& page = getOrCreatePage(index);
        page[index % pageSize] = T(std::forward<Args>(args)...);
    }

    constexpr void insert(std::size_t index, const T& value)
    {
        auto& page = getOrCreatePage(index);
        page[index % pageSize] = value;
    }

private:
    constexpr std::vector<T>& getOrCreatePage(std::size_t index)
    {
        auto pageIndex = index / pageSize;
        if (pageIndex < pages.size() && pages[pageIndex].size() != 0)
            return pages[pageIndex];

        if (pageIndex >= pages.size())
            pages.resize(pageIndex + 1);

        //constexpr bool hasDefault = defaultValue == T{};
        return initPage(pages[pageIndex]);
    }

    constexpr std::vector<T>& initPage(std::vector<T>& page)
    {
        return page = std::vector<T>(pageSize, defaultValue);
    }

    // constexpr std::vector<T>& initPage(std::vector<T>& page, std::false_type)
    // {
    //     return page = std::vector<T>(pageSize);
    // }

private:
    std::vector<std::vector<T>> pages;
};

} // Plunksna

#endif //PAGINATEDVECTOR_H
