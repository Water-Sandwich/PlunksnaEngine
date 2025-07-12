//
// Created by d on 5/16/25.
//

#ifndef COMPONENTSTORE_H
#define COMPONENTSTORE_H

#include <vector>

#include "Entity.h"
#include "PaginatedVector.h"

namespace Plunksna {

//Interface
class IComponentStore
{
public:
    virtual ~IComponentStore() = default;

    //remove component from entity, returns the entity and pointer of component if there was a move operation
    virtual std::pair<Entity, void*> remove(Entity entity) = 0;

    //get offset of component vector after a potential whole vector move operation
    virtual std::ptrdiff_t offsetAfterMove() = 0;

    //get address of component from entity
    void* at(Entity entity);

    virtual std::size_t count() const = 0;

protected:
    virtual void* atImpl(Entity entity) = 0;

public:
    ComponentMask m_bitmask;
};

template <typename Component>
class ComponentStore final : public IComponentStore{
public:
    explicit ComponentStore(std::size_t reserveSize = 512) noexcept;

    //get component from entity
    Component* get(Entity entity) const;

    //construct a component for entity
    template<typename... Args>
    bool add(Entity entity, Args&&... args);

    //remove component from entity
    std::pair<Entity, void*> remove(Entity entity) override;

    //calculate how far the vector has moved due to a resize
    std::ptrdiff_t offsetAfterMove() override;

    //return the amount of components in this store
    std::size_t count() const override;

private:
    void* atImpl(Entity entity) override;

private:
    //TODO: Paginate this
    PaginatedVector<std::size_t, NULL_INDEX> m_indexes; // Sparse
    std::vector<Component> m_components; // Dense
    std::vector<Entity> m_entities; // Dense
    void* m_data = nullptr;
    void* m_first = nullptr;
};

} // Plunksna

#include "ComponentStore.tpp"

#endif //COMPONENTSTORE_H
