//
// Created by d on 5/16/25.
//
#pragma once

#include "ComponentStore.h"

namespace Plunksna {

template <typename Component>
ComponentStore<Component>::ComponentStore(std::size_t reserveSize) noexcept
{
    m_components.reserve(reserveSize);
    m_owners.reserve(reserveSize);
}

template <typename Component>
Component* ComponentStore<Component>::get(Entity entity) const
{
    if (std::size_t index = m_indexes[entity]; index != NULL_INDEX)
        return &m_components[index];
    return nullptr;
}

template <typename Component>
template <typename ... Args>
bool ComponentStore<Component>::add(Entity entity, Args&&... args)
{
    if (entity == NULL_ENTITY) {
        LOG_S(eWARNING, "adding to a null entity")
        return false;
    }

    m_components.emplace_back(std::forward<Args>(args)...);
    m_owners.emplace_back(entity);
    m_indexes.insert(entity, m_components.size() - 1);

    return true;
}

template <typename Component>
bool ComponentStore<Component>::remove(Entity entity)
{
    if (entity == NULL_ENTITY) {
        LOG_S(eWARNING, "removing a null entity")
        return false;
    }

    auto& index = m_indexes[entity];

    if (index == NULL_INDEX)
        return false;

    std::swap(m_components[index], m_components.back());
    std::swap(m_owners[index], m_owners.back());

    m_components.pop_back();
    m_owners.pop_back();
    index = NULL_INDEX;

    return true;
}

} // Plunksna