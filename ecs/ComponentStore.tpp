//
// Created by d on 5/16/25.
//
#pragma once

#include "ComponentStore.h"

namespace Plunksna {

template <typename Component>
Component* ComponentStore<Component>::get(Entity entity)
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
        LOG_S(eWARNING, "Max entity number reached")
        return false;
    }

    m_components.emplace_back(std::forward<Args>(args)...);
    m_owners.emplace_back(entity);

    // if (m_indexes.size() <= entity)
    //     m_indexes.resize(entity + 1);
    //
    // m_indexes[entity] = m_components.size() - 1;
    m_indexes.insert(entity, m_components.size() - 1);
    return true;
}

template <typename Component>
bool ComponentStore<Component>::remove(Entity entity)
{
    if (entity == NULL_ENTITY) {
        LOG_S(eWARNING, "Max entity number reached")
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