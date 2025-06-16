//
// Created by d on 5/16/25.
//

#ifndef COMPONENTSTORE_H
#define COMPONENTSTORE_H

#include <vector>
#include <limits>

#include "Entity.h"
#include "PaginatedVector.h"
#include "../engine/Log.h"

namespace Plunksna {

//Interface
class IComponentStore
{
public:
    virtual ~IComponentStore() = default;
    virtual std::pair<Entity, void*> remove(Entity entity) = 0;
public:
    ComponentMask bitmask;
};

template <typename Component>
class ComponentStore final : public IComponentStore{
public:
    ComponentStore() noexcept = default;
    explicit ComponentStore(std::size_t reserveSize) noexcept;

    //get component from entity
    Component* get(Entity entity) const;

    //construct a component for entity
    template<typename... Args>
    bool add(Entity entity, Args&&... args);

    //remove component from entity
    std::pair<Entity, void*> remove(Entity entity) override;

private:
    //TODO: Paginate this
    PaginatedVector<std::size_t, NULL_INDEX> m_indexes; // Sparse
    std::vector<Component> m_components; // Dense
    std::vector<Entity> m_entities; // Dense
};

} // Plunksna

#include "ComponentStore.tpp"

#endif //COMPONENTSTORE_H
