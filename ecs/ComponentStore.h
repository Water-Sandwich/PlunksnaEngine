//
// Created by d on 5/16/25.
//

#ifndef COMPONENTSTORE_H
#define COMPONENTSTORE_H

#include <vector>
#include <limits>

#include "PaginatedVector.h"
#include "../engine/Log.h"

namespace Plunksna {

using Entity = unsigned long int;
constexpr Entity NULL_ENTITY = std::numeric_limits<Entity>::max();
constexpr Entity NULL_INDEX = std::numeric_limits<std::size_t>::max();

//Interface
class IComponentStore
{
public:
    virtual ~IComponentStore() = default;
    virtual bool remove(Entity entity) = 0;
};

template <typename Component>
class ComponentStore final : public IComponentStore{
public:
    ComponentStore() noexcept = default;

    Component* get(Entity entity)
    {
        if (std::size_t index = m_indexes[entity]; index != NULL_INDEX)
            return &m_components[index];
        return nullptr;
    }

    template<typename... Args>
    void add(Entity entity, Args&&... args)
    {
        if (entity == NULL_ENTITY) {
            LOG_S(eWARNING, "Max entity number reached")
            return;
        }

        m_components.emplace_back(std::forward<Args>(args)...);
        m_owners.emplace_back(entity);

        // if (m_indexes.size() <= entity)
        //     m_indexes.resize(entity + 1);
        //
        // m_indexes[entity] = m_components.size() - 1;
        m_indexes.insert(entity, m_components.size() - 1);
    }

    bool remove(Entity entity) override
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

private:
    //TODO: Paginate this
    PaginatedVector<std::size_t, NULL_INDEX> m_indexes; // Sparse
    std::vector<Component> m_components; // Dense
    std::vector<Entity> m_owners; // Dense
};

} // Plunksna

#endif //COMPONENTSTORE_H
