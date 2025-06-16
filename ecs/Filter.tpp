//
// Created by d on 6/12/25.
//
#ifndef FILTER_TPP
#define FILTER_TPP

#include "Filter.h"

namespace Plunksna {

template <typename ... Components>
Filter<Components...>::Filter(FilterFunction function)
    : m_function(function)
{
    if (s_offsetsPerType.empty()) {
        calculateOffsets();
    }

    m_entities.reserve(RESERVE_SIZE);
    m_components.reserve(RESERVE_SIZE);
}

template <typename ... Components>
bool Filter<Components...>::foreach(FilterFunction function)
{
    if (!function)
        return false;

    for (const std::tuple<Components*...>& componentTuple : m_components) {
        std::apply([&](Components*... ptrs)
        {
            function(*ptrs...);
        }, componentTuple);
    }

    return true;
}

template <typename ... Components>
bool Filter<Components...>::add(Entity entity, Components*... components)
{
    if (entity == NULL_ENTITY)
        return false;

    if ((... || (components == nullptr)))
        return false;

    m_entities.push_back(entity);
    m_components.emplace_back(components...);
    m_indexes.insert(entity, m_entities.size() - 1);

    return true;
}

template <typename ... Components>
std::pair<Entity, void*> Filter<Components...>::remove(Entity entity)
{
    if (entity == NULL_ENTITY) {
        LOG_S(eWARNING, "removing a null entity")
        return {NULL_ENTITY, nullptr};
    }

    const auto index = m_indexes[entity];

    if (index == NULL_INDEX)
        return {NULL_ENTITY, nullptr};

    const auto otherIndex = m_indexes[m_entities.back()];
    const auto otherEntity = m_entities[otherIndex];

    std::swap(m_components[m_indexes[entity]], m_components.back());
    std::swap(m_entities[m_indexes[entity]], m_entities.back());

    m_components.pop_back();
    m_entities.pop_back();

    m_indexes[otherEntity] = m_indexes[entity];
    m_indexes[entity] = NULL_ENTITY;

    if (m_entities.empty()) {
        LOG("No more components of this type")
        return {NULL_ENTITY, nullptr};
    }

    return {m_entities[m_indexes[otherIndex]],
            &m_components[m_indexes[otherIndex]]};
}

template <typename ... Components>
template <typename Component>
bool Filter<Components...>::updateComponentAddress(Entity entity, Component* address)
{
    if (!s_offsetsPerType.contains(typeid(Component)) || !address)
        return false;

    if (m_indexes[entity] == NULL_INDEX)
        return false;

    auto* start = reinterpret_cast<std::byte*>(&m_components[m_indexes[entity]]);
    const auto offset = s_offsetsPerType.at(typeid(Component));

    auto** component = reinterpret_cast<Component**>(start + offset);
    *component = address;

    return true;
}

template <typename ... Components>
bool Filter<Components...>::updateComponentAddress(Entity entity, std::type_index type, void* address)
{
    if (!s_offsetsPerType.contains(type) || !address)
        return false;

    if (m_indexes[entity] == NULL_INDEX)
        return false;

    auto* start = reinterpret_cast<std::byte*>(&m_components[m_indexes[entity]]);
    const auto offset = s_offsetsPerType.at(type);

    auto** component = reinterpret_cast<void**>(start + offset);
    *component = address;

    return true;
}

template <typename ... Components>
template <typename Component>
bool Filter<Components...>::updateComponentAddressFast(Entity entity, Component* address)
{
    if (m_indexes[entity] == NULL_INDEX)
        return false;

    auto& tuple = m_components[m_indexes[entity]];
    constexpr std::size_t index = tupleTypeIndex<Component, Components...>();
    std::get<index>(tuple) = address;

    return true;
}

template <typename ... Components>
bool Filter<Components...>::callForeach()
{
    if (!m_function)
        return false;

    for (const std::tuple<Components*...>& componentTuple : m_components) {
        std::apply([&](Components*... ptrs)
        {
            m_function(*ptrs...);
        }, componentTuple);
    }

    return true;
}

template <typename ... Components>
template <typename Component>
consteval std::size_t Filter<Components...>::tupleTypeIndex() {
    std::size_t index = 0;
    ((std::is_same_v<Component, Components> ? true : (++index, false)) || ...);
    return index;
}

template <typename ... Components>
constexpr void Filter<Components...>::calculateOffsets()
{
    std::tuple<Components*...> tup;
    auto* start = reinterpret_cast<std::byte*>(&tup);

    constexpr std::size_t count = sizeof...(Components);
    calculateOffsetsFold(std::make_index_sequence<count>{}, tup, start);
}

template <typename ... Components>
template <std::size_t... Count>
constexpr void Filter<Components...>::calculateOffsetsFold(std::index_sequence<Count...>, std::tuple<Components*...>& tup, std::byte* base)
{
    ((
        s_offsetsPerType[std::type_index(typeid(std::tuple_element_t<Count, std::tuple<Components...>>))] =
            reinterpret_cast<std::byte*>(&std::get<Count>(tup)) - base
    ), ...);
}


inline bool IFilter::foreachDefault()
{
    return callForeach();
}

} // Plunksna

#endif // FILTER_TPP
