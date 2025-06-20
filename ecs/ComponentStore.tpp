//
// Created by d on 5/16/25.
//
#ifndef COMPONENTSTORE_TPP
#define COMPONENTSTORE_TPP

#include "ComponentStore.h"

namespace Plunksna {

template <typename Component>
ComponentStore<Component>::ComponentStore(std::size_t reserveSize) noexcept
{
    m_components.reserve(reserveSize);
    m_entities.reserve(reserveSize);
    m_address = &*m_components.begin();
}

template <typename Component>
Component* ComponentStore<Component>::get(Entity entity) const
{
    if (std::size_t index = m_indexes[entity]; index != NULL_INDEX)
        //const cast required due to filter makeTuple
        return const_cast<Component*>(&m_components[index]);
    return nullptr;
}

template <typename Component>
template <typename ... Args>
bool ComponentStore<Component>::add(Entity entity, Args&&... args)
{
    if (entity == NULL_ENTITY) {
        LOG_S(eWARNING, "adding a null entity")
        return false;
    }

    m_components.emplace_back(std::forward<Args>(args)...);
    m_entities.emplace_back(entity);
    m_indexes.insert(entity, m_components.size() - 1);

    return true;
}

template <typename Component>
std::pair<Entity, void*> ComponentStore<Component>::remove(Entity entity)
{
    if (entity == NULL_ENTITY) {
        LOG_S(eWARNING, "removing a null entity")
        return {NULL_ENTITY, nullptr};
    }

    auto index = m_indexes[entity];

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

    index = m_indexes[otherIndex];

    if (index == NULL_INDEX)
        return {NULL_ENTITY, nullptr};

    if (m_entities.empty()) {
        LOG("No more components of this type")
        return {NULL_ENTITY, nullptr};
    }

    return {m_entities[index],
            &m_components[index]};
}

template <typename Component>
std::size_t ComponentStore<Component>::offsetAfterMove()
{
    void* startPtr = &*m_components.begin();
    const std::size_t distance = reinterpret_cast<std::size_t>(startPtr) - reinterpret_cast<std::size_t>(m_address);
    m_address = startPtr;
    return distance;
}

template <typename Component>
std::size_t ComponentStore<Component>::count() const
{
    return m_components.size();
}

template <typename Component>
void* ComponentStore<Component>::atImpl(Entity entity)
{
    auto index = m_indexes[entity];

    if (index == NULL_INDEX)
        return nullptr;

    return &m_components[index];
}

inline void* IComponentStore::at(Entity entity)
{
    return atImpl(entity);
}

} // Plunksna
#endif // COMPONENTSTORE_TPP
