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

  LoadedEntity( const Entity&                             entity
              , const std::tuple<ComponentStore<Ts>*...>& stores)
    : _entity(entity)
    , _stores(stores)
  {
    assert(detail::all(stores, [&](auto store) {
      return entity.container().contains_store(*store);
    }));
  }

  // Convert LoadedEntity<Us...> to LoadedEntity<Ts...> only if Ts... is subset
  // of Us...
  template<typename... Us>
  LoadedEntity(
      const LoadedEntity<Us...>& other
    , std::enable_if_t< detail::IsSubset< std::tuple<Ts...>
                                        , std::tuple<Us...>>>* = nullptr)
    : LoadedEntity(
        other._entity
      , std::make_tuple(std::get<ComponentStore<Ts>*>(other._stores)...))
  {}

  // Assign LoadedEntity<Us...> to LoadedEntity<Ts...> only if Ts... is subset
  // of Us...
  template<typename... Us>
  std::enable_if_t< detail::IsSubset<std::tuple<Ts...>, std::tuple<Us...>>
                  , LoadedEntity<Ts...>&>
  operator = (const LoadedEntity<Us...>& other) {
    _entity = other._entity;
    _stores = std::make_tuple(std::get<ComponentStore<Ts>*>(other._stores)...);
    return *this;
  }

  explicit operator bool () const {
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
    return { _entity.copy(), _stores };
  }

  LoadedEntity<Ts...> copy_to(Container& target) const {
    return { _entity.copy_to(target), target.store_ptrs<Ts...>() };
  }

  void destroy() const {
    _entity.destroy();
  }

  template<typename... Us>
  LoadedEntity<Ts..., Us...>
  load() const {
    static_assert( detail::IsDisjoint<std::tuple<Ts...>, std::tuple<Us...>>
                 , "Ts... and Us... cannot contain the same type");

    return { _entity
           , std::tuple_cat( _stores
                           , container().template store_ptrs<Us...>()) };
  }

  template<typename... Us>
  LoadedEntity<Ts..., Us...>
  load(const std::tuple<ComponentStore<Us>*...>& more_stores) const {
    static_assert( detail::IsDisjoint<std::tuple<Ts...>, std::tuple<Us...>>
                 , "Ts... and Us... cannot contain the same type");

    return { _entity, std::tuple_cat(_stores, more_stores) };
  }

private:
  Entity                             _entity;
  std::tuple<ComponentStore<Ts>*...> _stores;

  friend class Entity;
  template<typename...> friend class LoadedEntity;

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