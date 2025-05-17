//
// Created by d on 5/16/25.
//

#ifndef REGISTRY_H
#define REGISTRY_H

#include <memory>
#include <typeindex>
#include <unordered_map>

#include "ComponentStore.h"

namespace Plunksna {

class Registry {
public:
    template<typename Component, typename... Args>
    void add(Entity entity, Args&&... args)
    {
        ComponentStore<Component>& store = getOrCreateStore<Component>();
        store.add(entity, std::forward<Args>(args)...);
    }

    //returns true on success, else false
    template<typename Component>
    bool remove(Entity entity)
    {
        if (m_stores.count(typeid(Component)) == 0)
            return false;

        auto& store = getStore<Component>();
        return store.remove(entity);
    }

    template<typename Component>
    Component* get(Entity entity)
    {
        if (m_stores.count(typeid(Component)) == 0)
            return nullptr;

        auto& store = getStore<Component>();
        return store.get(entity);
    }

private:
    template<typename Component>
    ComponentStore<Component>& getOrCreateStore()
    {
        if (m_stores.count(typeid(Component)) != 0)
            return getStore<Component>();

        auto* t_storePtr = new ComponentStore<Component>();
        std::unique_ptr<IComponentStore> t_store(t_storePtr);
        m_stores[typeid(Component)] = std::move(t_store);

        return *static_cast<ComponentStore<Component>*>(t_storePtr);
    }

    template<typename Component>
    ComponentStore<Component>& getStore()
    {
        return *static_cast<ComponentStore<Component>*>(m_stores.at(typeid(Component)).get());
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStore>> m_stores;
};

} // Plunksna

#endif //REGISTRY_H
