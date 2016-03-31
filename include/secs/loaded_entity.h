#pragma once

// Entity with pre-loaded Components.

#include "secs/container.h"
#include "secs/detail.h"
#include "secs/entity.h"

namespace secs {

template<typename... Ts>
class LoadedEntity {
public:
  LoadedEntity() {}

  LoadedEntity(const Entity& entity)
    : LoadedEntity(entity, entity.container().store_ptrs<Ts...>())
  {}

  LoadedEntity( const Entity&                             entity
              , const std::tuple<ComponentStore<Ts>*...>& stores)
    : _entity(entity)
    , _stores(stores)
  {
    assert(detail::all(stores, [&](auto store) {
      return entity.container().contains_store(*store);
    }));
  }

  // template<typename... Us>
  // LoadedEntity()

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
    return Entity::component<T>();
  }

  template<typename U0, typename U1, typename... Us>
  bool contains_all() const {
    return contains_all<U0>() && contains_all<U1, Us...>();
  }

  template<typename U>
  bool contains_all() const {
    return (bool) component<U>();
  }

  template<typename U0, typename U1, typename... Us>
  bool contains_none() const {
    return contains_none<U0>() && contains_none<U1, Us...>();
  }

  template<typename U>
  bool contains_none() const {
    return ! (bool) component<U>();
  }

  Container& container() const {
    return _entity.container();
  }

  LoadedEntity<Ts...> copy() const {
    return { Entity::copy(), _stores };
  }

  LoadedEntity<Ts...> copy_to(Container& target) const {
    return { Entity::copy_to(target) };
  }

  void destroy() const {
    _entity.destroy();
  }

  template<typename... Us>
  LoadedEntity<Ts..., Us...>
  load(const std::tuple<ComponentStore<Us>*...>& more_stores) const {
    static_assert( detail::IsDisjoint<std::tuple<Ts...>, std::tuple<Us...>>
                 , "Cannot load already loaded type");

    return { _entity, std::tuple_cat(_stores, more_stores) };
  }

private:
  Entity                             _entity;
  std::tuple<ComponentStore<Ts>*...> _stores;

  template<typename... As, typename... Bs>
  friend bool operator == ( const LoadedEntity<As...>&
                          , const LoadedEntity<Bs...>&);

  template<typename... As, typename... Bs>
  friend bool operator < ( const LoadedEntity<As...>&
                         , const LoadedEntity<Bs...>&);

};

template<typename... As, typename... Bs>
bool operator == (const LoadedEntity<As...>& a, const LoadedEntity<Bs...>& b) {
  return a._entity == b._entity;
}

template<typename... As, typename... Bs>
bool operator != (const LoadedEntity<As...>& a, const LoadedEntity<Bs...>& b) {
  return !(a == b);
}

template<typename... As, typename... Bs>
bool operator < (const LoadedEntity<As...>& a, const LoadedEntity<Bs...>& b) {
  return a._entity < b._entity;
}

}