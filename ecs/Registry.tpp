//
// Created by d on 5/16/25.
//
#pragma once

#include "Registry.h"

namespace Plunksna {

template <typename Component, typename ... Args>
bool Registry::add(Entity entity, Args&&... args)
{
    ComponentStore<Component>& store = getOrCreateStore<Component>();
    return store.add(entity, std::forward<Args>(args)...);
}

template<typename Component>
bool Registry::remove(Entity entity)
{
    if (!m_stores.contains(typeid(Component)))
        return false;

    auto& store = getStore<Component>();
    return store.remove(entity);
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

    auto* t_storePtr = new ComponentStore<Component>();
    std::unique_ptr<IComponentStore> t_store(t_storePtr);
    m_stores[typeid(Component)] = std::move(t_store);

    return *static_cast<ComponentStore<Component>*>(t_storePtr);
}

template <typename Component>
ComponentStore<Component>& Registry::getStore()
{
    return *static_cast<ComponentStore<Component>*>(m_stores.at(typeid(Component)).get());
}

inline Entity Registry::makeEntity()
{
    return m_maxEntity++;
}

} // Plunksna