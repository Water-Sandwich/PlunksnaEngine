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
    //get an entityID
    Entity makeEntity();

    //add a component to an entity
    //returns true on success, false on failure
    template<typename Component, typename... Args>
    bool add(Entity entity, Args&&... args);

    //remove a component from an entity
    //returns true on success, else false
    template<typename Component>
    bool remove(Entity entity);

    //get a pointer to an entity's component
    template<typename Component>
    Component* get(Entity entity);

private:
    template<typename Component>
    ComponentStore<Component>& getOrCreateStore();

    template<typename Component>
    ComponentStore<Component>& getStore();

private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStore>> m_stores;
    Entity m_maxEntity = 0;
};

} // Plunksna

#include "Registry.tpp"

#endif //REGISTRY_H
