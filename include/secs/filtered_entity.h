#pragma once

// Result of iterating over EntityFilter. Has the same public API as Entity and
// implicitly converts to it.

#include "secs/container.h"
#include "secs/detail.h"
#include "secs/entity.h"

namespace secs {

template<typename... Ts>
class FilteredEntity {
public:
  explicit operator bool () const noexcept {
    return (bool) _entity;
  }

  template<typename T>
  std::enable_if_t<detail::Contains<T, Ts...>, ComponentPtr<T>>
  component() const {
    auto store = std::get<ComponentStore<T>*>(_stores);

    assert(store);
    assert(_entity);

    return { *store, _entity._index, _entity._version };
  }

  template<typename T>
  std::enable_if_t<!detail::Contains<T, Ts...>, ComponentPtr<T>>
  component() const {
    return _entity.component<T>();
  }

  template<typename T, typename... Args>
  ComponentPtr<T> create_component(Args&&... args) const {
    return _entity.create_component<T, Args...>(std::forward<Args>(args)...);
  }

  template<typename T>
  void destroy_component() const {
    _entity.destroy_component<T>();
  }

  Container& container() const {
    return _entity.container();
  }

  FilteredEntity<Ts...> copy() const {
    return { _entity.copy(), _stores };
  }

  FilteredEntity<Ts...> copy_to(Container& target) const {
    return { _entity.copy_to(target), target.store_ptrs<Ts...>() };
  }

  void destroy() const {
    _entity.destroy();
  }

private:
  FilteredEntity( const Entity&                             entity
                , const std::tuple<ComponentStore<Ts>*...>& stores)
    : _entity(entity)
    , _stores(stores)
  {}

private:
  Entity                             _entity;
  std::tuple<ComponentStore<Ts>*...> _stores;

  friend class Entity;
  template<typename, typename...> friend class EntityFilter;

  template<typename... As, typename... Bs>
  friend bool operator == ( const FilteredEntity<As...>&
                          , const FilteredEntity<Bs...>&);

  template<typename... As, typename... Bs>
  friend bool operator < ( const FilteredEntity<As...>&
                         , const FilteredEntity<Bs...>&);
};

template<typename... As, typename... Bs>
bool operator == ( const FilteredEntity<As...>& a
                 , const FilteredEntity<Bs...>& b)
{
  return a._entity == b._entity;
}

template<typename... As, typename... Bs>
bool operator != ( const FilteredEntity<As...>& a
                 , const FilteredEntity<Bs...>& b)
{
  return !(a == b);
}

template<typename... As, typename... Bs>
bool operator < ( const FilteredEntity<As...>& a
                , const FilteredEntity<Bs...>& b)
{
  return a._entity < b._entity;
}

}