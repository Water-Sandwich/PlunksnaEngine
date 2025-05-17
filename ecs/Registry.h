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

private:
    template<typename Component>
    ComponentStore<Component>& getOrCreateStore()
    {
        if (m_stores.count(typeid(Component)) != 0)
            return *static_cast<ComponentStore<Component>*>(m_stores.at(typeid(Component)).get());

        auto* t_storePtr = new ComponentStore<Component>();
        std::unique_ptr<IComponentStore> t_store(new ComponentStore<Component>());
        m_stores[typeid(Component)] = std::move(t_store);

        return *static_cast<ComponentStore<Component>*>(t_storePtr);
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStore>> m_stores;
};

} // Plunksna

#endif //REGISTRY_H
