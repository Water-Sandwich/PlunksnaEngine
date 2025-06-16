//
// Created by d on 6/12/25.
//

#ifndef FILTER_H
#define FILTER_H

#include <tuple>
#include <vector>

#include "Entity.h"

namespace Plunksna {

class IFilter {
public:
    //call the stored function pointer on each tuple (if one exists)
    bool foreachDefault();

    //remove an entity from the filter
    virtual std::pair<Entity, void*> remove(Entity entity) = 0;

    //update a certain component's location in this filter
    virtual bool updateComponentAddress(Entity entity, std::type_index type, void* address) = 0;

    virtual ~IFilter() = default;

protected:
    //implementation for foreachDefault
    virtual bool callForeach() = 0;

public:
    ComponentMask bitmask;

protected:
    inline static std::unordered_map<std::type_index, std::size_t> s_offsetsPerType;
    PaginatedVector<std::size_t, NULL_INDEX> m_indexes;
    std::vector<Entity> m_entities;
};

//helpers to make sure each component is unique
template<typename...>
struct are_unique : std::true_type {};

template<typename T, typename... Rest>
struct are_unique<T, Rest...>
    : std::conjunction<
          std::negation<std::disjunction<std::is_same<T, Rest>...>>,
          are_unique<Rest...>
      > {};

template <typename... Components>
class Filter final : public IFilter
{
    static_assert(are_unique<Components...>::value, "Filter components must be unique)");
    using FilterFunction = void(*)(Components&...);

public:
    explicit Filter(FilterFunction function = nullptr);
    Filter() = delete;

    //run a function on every entity in the filter
    bool foreach(FilterFunction function);

    //add an entity with its respective components to the filter
    bool add(Entity entity, Components*... components);
    //remove and entity
    std::pair<Entity, void*> remove(Entity entity) override;

    //update the location of a certain component for an entity
    template<typename Component>
    bool updateComponentAddress(Entity entity, Component* address);

    bool updateComponentAddress(Entity entity, std::type_index type, void* address) override;

    //update the location of a certain component for an entity, less checking and uses compile time information
    template<typename Component>
    bool updateComponentAddressFast(Entity entity, Component* address);

private:
    template <typename Component>
    static consteval std::size_t tupleTypeIndex();

    bool callForeach() override;
    constexpr static void calculateOffsets();

    template <std::size_t... Count>
    constexpr static void calculateOffsetsFold(std::index_sequence<Count...>, std::tuple<Components*...>& tup, std::byte* base);

private:
    FilterFunction m_function;
    std::vector<std::tuple<Components*...>> m_components; //TODO: should be shared/weak pointers
    static constexpr std::size_t RESERVE_SIZE = 512;
};

} // Plunksna

#include "Filter.tpp"

#endif //FILTER_H
