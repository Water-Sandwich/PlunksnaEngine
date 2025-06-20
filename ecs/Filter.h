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
    using FilterDeleter = void(*)(void*);

public:
    //call the stored function pointer on each tuple (if one exists)
    bool foreachDefault();

    //remove an entity from the filter
    virtual std::pair<Entity, void*> remove(Entity entity) = 0;

    //check if entity is in filter
    virtual bool has(Entity entity) = 0;

    //add an entity with a pre-existing tuple
    virtual bool add(Entity entity, void* tuple) = 0;

    virtual std::unique_ptr<void, FilterDeleter> makeTuple(const std::unordered_map<std::type_index, std::unique_ptr<IComponentStore>>& registryStores, Entity entity);

    //update a certain component's location in this filter
    virtual bool updateComponentAddress(Entity entity, std::type_index type, void* address) = 0;

    //update locations of all component pointers after a move operation from registry
    bool updateAllComponentAddresses(std::size_t offset, std::type_index type);

    //return the amount of components this filter has
    virtual std::size_t count() const = 0;

    virtual ~IFilter() = default;

protected:
    //implementation for foreachDefault
    virtual bool foreachImpl() = 0;

    virtual bool updateAllComponentAddressesImpl(std::size_t offset, std::type_index type) = 0;

    virtual std::unique_ptr<void, FilterDeleter> makeTupleImpl(const std::unordered_map<std::type_index, std::unique_ptr<IComponentStore>>& registryStores, Entity entity) = 0;

public:
    ComponentMask m_bitmask;

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
public:
    using FilterFunction = void(*)(Components&...);
    static_assert(are_unique<Components...>::value, "Filter components must be unique");

public:
    explicit Filter(FilterFunction function = nullptr, std::size_t reserveSize = FILTER_RESERVE_SIZE);
    Filter() = delete;

    //run a function on every entity in the filter
    bool foreach(FilterFunction function);

    //returns if an entity exists in the filter
    bool has(Entity entity) override;

    //add an entity with its respective components to the filter
    bool add(Entity entity, Components*... components);

    bool add(Entity entity, void* tuple) override;

    //remove and entity
    std::pair<Entity, void*> remove(Entity entity) override;

    //update the location of a certain component for an entity
    template<typename Component>
    bool updateComponentAddress(Entity entity, Component* address);
    bool updateComponentAddress(Entity entity, std::type_index type, void* address) override;

    //update the location of a certain component for an entity, less checking and uses compile time information
    template<typename Component>
    bool updateComponentAddressFast(Entity entity, Component* address);

    std::size_t count() const override;

private:
    template <typename Component>
    static consteval std::size_t tupleTypeIndex();

    bool foreachImpl() override;
    constexpr static void calculateOffsets();

    std::unique_ptr<void, FilterDeleter> makeTupleImpl(const std::unordered_map<std::type_index, std::unique_ptr<IComponentStore>>& registryStores, Entity entity) override;

    template <std::size_t... Count>
    constexpr static void calculateOffsetsFold(std::index_sequence<Count...>, std::tuple<Components*...>& tup, std::byte* base);

    bool updateAllComponentAddressesImpl(std::size_t offset, std::type_index type) override;

    template <typename Component>
    Component* getAddressFromStore(const std::unordered_map<std::type_index, std::unique_ptr<IComponentStore>>& registryStores, Entity entity);

private:
    FilterFunction m_function;
    std::vector<std::tuple<Components*...>> m_components;

    static constexpr FilterDeleter s_deleter = [](void* ptr)
    {
        delete static_cast<std::tuple<Components*...>*>(ptr);
    };
};

} // Plunksna

#include "Filter.tpp"

#endif //FILTER_H
