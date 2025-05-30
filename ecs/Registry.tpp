//
// Created by d on 5/16/25.
//
#pragma once

#include "Registry.h"
#include "../engine/Exception.h"

namespace Plunksna {

template <typename Component, typename ... Args>
bool Registry::add(Entity entity, Args&&... args)
{
    //if unsuccessful, return out
    if (!getOrCreateStore<Component>().add(entity, std::forward<Args>(args)...))
        return false;

    setMaskBit<Component>(entity, true);

    return true;
}

template<typename Component>
bool Registry::remove(Entity entity)
{
    if (!m_stores.contains(typeid(Component)))
        return false;

    //if unsuccessful, return out
    if (!getStore<Component>().remove(entity))
        return false;

    setMaskBit<Component>(entity, false);

    return true;
}

template <typename Component>
Component* Registry::get(Entity entity)
{
    if (!m_stores.contains(typeid(Component)))
        return nullptr;

    auto& store = getStore<Component>();
    return store.get(entity);
}

template <typename Component>
ComponentStore<Component>& Registry::getOrCreateStore()
{
    if (m_stores.contains(typeid(Component)))
        return getStore<Component>();

    auto* storePtr = new ComponentStore<Component>();
    storePtr->bitmask.set(m_stores.size(), true);

    std::unique_ptr<IComponentStore> store(storePtr);
    m_stores[typeid(Component)] = std::move(store);

    THROW_IF_EQ(m_componentTypes.size() == MAX_COMPONENTS, "Max components reached!");
    m_componentTypes.emplace_back(typeid(Component));

    return *static_cast<ComponentStore<Component>*>(storePtr);
}

template <typename Component>
ComponentStore<Component>& Registry::getStore()
{
    return *static_cast<ComponentStore<Component>*>(m_stores.at(typeid(Component)).get());
}

template <typename Component>
void Registry::setMaskBit(Entity entity, bool value)
{
    auto index = findIndexOfEntity(entity);
    auto typeIt = std::find(m_componentTypes.begin(), m_componentTypes.end(), typeid(Component)); //TODO: Make a map for type -> index?
    auto typeIndex = std::distance(m_componentTypes.begin(), typeIt);
    m_entities.componentMasks[index].set(typeIndex, value);
}

inline Registry::Registry(std::size_t reserveSize) noexcept
{
    m_componentTypes.reserve(MAX_COMPONENTS);
    m_entities.entities.reserve(reserveSize);
    m_entities.componentMasks.reserve(reserveSize);
}

inline Entity Registry::makeEntity()
{
    m_entities.entities.emplace_back(m_maxEntity++);
    m_entities.componentMasks.emplace_back();
    return m_entities.entities.back();
}

inline bool Registry::removeEntity(Entity entity)
{
    auto index = findIndexOfEntity(entity);

    if (index == NULL_INDEX)
        return false;

    std::swap(m_entities.entities[index], m_entities.entities.back());
    std::swap(m_entities.componentMasks[index], m_entities.componentMasks.back());

    //TODO: Remove components of entity
    for (int i = 0; i < m_componentTypes.size(); i++) {
        if (m_entities.componentMasks.back()[i] == 0)
            continue;

        getStore(m_componentTypes[i])->remove(m_entities.entities.back());
    }

    m_entities.entities.pop_back();
    m_entities.componentMasks.pop_back();

    return true;
}

inline IComponentStore* Registry::getStore(std::type_index type)
{
    return m_stores.at(type).get();
}

inline std::size_t Registry::findIndexOfEntity(Entity entity)
{
    static constexpr auto comp = [](const Entity& a, const Entity& b){return a < b;};
    auto it = std::lower_bound(m_entities.entities.begin(), m_entities.entities.end(), entity, comp);

    if(it == m_entities.entities.end())
        return NULL_INDEX;

    return std::distance(m_entities.entities.begin(), it);
}

} // Plunksna