//
// Created by d on 5/16/25.
//

#ifndef REGISTRY_H
#define REGISTRY_H

#include <memory>
#include <typeindex>
#include <unordered_map>

#include "ComponentStore.h"
#include "Filter.h"

namespace Plunksna {

struct EntityDesc
{
    std::vector<Entity> entities;
    std::vector<ComponentMask> componentMasks;
    std::vector<std::size_t> fragmentIndexes;
};

class Registry final {
public:
    explicit Registry(std::size_t reserveSize = 512) noexcept;

    //get an entityID
    Entity makeEntity();

    //remove an entity and its components
    bool removeEntity(Entity entity);

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

    IComponentStore* getStore(std::type_index type) const;

    std::size_t findIndexOfEntity(Entity entity);

    template<typename Component>
    void setMaskBit(Entity entity, bool value);

private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStore>> m_stores;
    //maps index -> type
    std::vector<std::type_index> m_componentTypes;

    EntityDesc m_entities;
    Entity m_maxEntity = 0;

    //Filter<int,int,float> filter;
};

} // Plunksna

#include "Registry.tpp"

#endif //REGISTRY_H
