//
// Created by d on 5/16/25.
//
#ifndef REGISTRY_TPP
#define REGISTRY_TPP

#include "Registry.h"
#include "../engine/Exception.h"

namespace Plunksna {

template <typename Component, typename ... Args>
bool Registry::add(Entity entity, Args&&... args)
{
    //if unsuccessful, return out
    ComponentStore<Component>& store = getOrCreateStore<Component>();

    if (bool ret = store.add(entity, std::forward<Args>(args)...); !ret)
        return false;

    //check if resize operation occured, if so, update filter pointers
    auto distance = store.offsetAfterMove();
    if (distance != 0)
        updateAllFilterAddresses(distance, store.m_bitmask, typeid(Component));

    auto bitmask = setMaskBit<Component>(entity, true);
    for (auto& filterPtr : m_filters) {
        auto& filter = *filterPtr;

        if ((filter.m_bitmask & bitmask) != filter.m_bitmask)
            continue;
        if (filterPtr->has(entity))
            continue;

        //retrieve all pointers of all components associated with this entity
        auto tup = filter.makeTuple(m_stores, entity);
        filter.add(entity, tup.get());
    }

    return true;
}

template<typename Component>
bool Registry::remove(Entity entity)
{
    if (!m_stores.contains(typeid(Component)))
        return false;

    auto& store = getStore<Component>();
    std::pair<Entity, void*> ret = store.remove(entity);
    auto bitmask = setMaskBit<Component>(entity, false);

    //remove entity from filters
    for (int i = 0; i < m_filters.size(); i++) {
        auto& filter = *m_filters[i].get();

        if ((filter.m_bitmask & bitmask) == filter.m_bitmask)
            continue;
        if (!filter.has(entity))
            continue;

        //retrieve all pointers of all components associated with this entity
        filter.remove(entity);
    }

    if (!ret.second)
        return false;

    //update moved components from removal
    for (int i = 0; i < m_filters.size(); i++) {
        auto& filter = *m_filters[i].get();

        if ((filter.m_bitmask & bitmask) != filter.m_bitmask)
            continue;
        if (filter.has(entity))
            continue;

        filter.updateComponentAddress(ret.first, typeid(Component), ret.second);
    }

    return true;
}

template <typename Component>
Component* Registry::get(Entity entity)
{
    if (!m_stores.contains(typeid(Component)))
        return nullptr;

    auto& store = getStore<Component>();
    return store.get(entity);
}

template <typename ... Components>
Filter<Components...>* Registry::makeFilter(typename Filter<Components...>::FilterFunction func, int priority, std::size_t reserveSize)
{
    auto filter = std::make_unique<Filter<Components...>>(func, priority, reserveSize);
    auto filterPtr = filter.get();

    std::tuple<ComponentStore<Components>...> stores = std::make_tuple(getOrCreateStore<Components>()...);
    std::apply([&](ComponentStore<Components>... store)
    {
        (filterPtr->m_bitmask |= ... |= store.m_bitmask);
    }, stores);

    auto it = std::lower_bound(m_filters.begin(), m_filters.end(), filterPtr, [](const std::unique_ptr<IFilter>& fil, const IFilter* const value)
    {
        return fil->m_priority < value->m_priority;
    });

    m_filters.insert(it, std::move(filter));

    //m_filters.push_back(std::move(filter));
    return filterPtr;
}

template <typename Component>
std::size_t Registry::count() const
{
    return getStore<Component>().count();
}

template <typename Component>
ComponentStore<Component>& Registry::getOrCreateStore()
{
    if (m_stores.contains(typeid(Component)))
        return getStore<Component>();

    auto* storePtr = new ComponentStore<Component>();
    storePtr->m_bitmask.set(m_stores.size(), true);

    std::unique_ptr<IComponentStore> store(storePtr);
    m_stores[typeid(Component)] = std::move(store);

    THROW_IF_EQ(m_componentTypes.size() == MAX_COMPONENTS, "Max components reached!");
    m_componentTypes.emplace_back(typeid(Component));

    return *static_cast<ComponentStore<Component>*>(storePtr);
}

template <typename Component>
ComponentStore<Component>& Registry::getStore()
{
    return *static_cast<ComponentStore<Component>*>(m_stores.at(typeid(Component)).get());
}

template <typename Component>
const ComponentMask& Registry::setMaskBit(Entity entity, bool value)
{
    const auto index = findIndexOfEntity(entity);
    const auto typeIt = std::find(m_componentTypes.begin(), m_componentTypes.end(), typeid(Component)); //TODO: Make a map for type -> index?
    const auto typeIndex = std::distance(m_componentTypes.begin(), typeIt);
    m_entities.componentMasks[index].set(typeIndex, value);
    return m_entities.componentMasks[index];
}

inline Registry::Registry(std::size_t reserveSize) noexcept
{
    m_componentTypes.reserve(MAX_COMPONENTS);
    m_entities.entities.reserve(reserveSize);
    m_entities.componentMasks.reserve(reserveSize);
    m_entities.fragmentIndexes.reserve(reserveSize);
}

inline Entity Registry::makeEntity()
{
    if (m_entities.fragmentIndexes.empty()) {
        m_entities.entities.emplace_back(m_maxEntity++);
        m_entities.componentMasks.emplace_back();
        return m_entities.entities.back();
    }

    const auto index = m_entities.fragmentIndexes.back();
    m_entities.fragmentIndexes.pop_back();
    return m_entities.entities[index];
}

inline bool Registry::removeEntity(Entity entity)
{
    const auto index = findIndexOfEntity(entity);

    if (index == NULL_INDEX)
        return false;

    //TODO: Remove components of entity
    for (int i = 0; i < m_componentTypes.size(); i++) {
        if (m_entities.componentMasks[index][i] == 0)
            continue;

        //make remove return the index of the moved object
        //update each filter with this component and other entity accordingly
        getStore(m_componentTypes[i])->remove(m_entities.entities[index]);
    }

    m_entities.fragmentIndexes.push_back(index);
    m_entities.componentMasks[index].reset();

    return true;
}

inline std::size_t Registry::totalCount() const
{
    return m_entities.entities.size();
}

inline IComponentStore* Registry::getStore(std::type_index type) const
{
    return m_stores.at(type).get();
}

inline std::size_t Registry::findIndexOfEntity(Entity entity)
{
    static constexpr auto comp = [](const Entity& a, const Entity& b){return a < b;};
    const auto it = std::lower_bound(m_entities.entities.begin(), m_entities.entities.end(), entity, comp);

    if(it == m_entities.entities.end())
        return NULL_INDEX;

    return std::distance(m_entities.entities.begin(), it);
}

inline void Registry::updateAllFilterAddresses(std::ptrdiff_t offset, ComponentMask mask, std::type_index type) const
{
    for (auto& filterPtr : m_filters) {
        auto& filter = *filterPtr;
        auto tmask = filter.m_bitmask & mask;
        if ((filter.m_bitmask & mask) != mask)
            continue;

        filter.updateAllComponentAddresses(offset, type);
    }
}

} // Plunksna
#endif // REGISTRY_TPP
