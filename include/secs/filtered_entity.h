#pragma once

// Result of iterating over EntityFilter. Has the same public API as Entity and
// implicitly converts to it.

#include "secs/container.h"
#include "secs/entity.h"

namespace secs {
namespace detail {

// Contains implementation
template<typename T, typename... Us>
struct ContainsImpl;

template<typename T, typename U, typename... Us>
struct ContainsImpl<T, U, Us...> {
  static const bool value = std::is_same<T, U>::value
                         || ContainsImpl<T, Us...>::value;
};

template<typename T>
struct ContainsImpl<T> {
  static const bool value = false;
};

// Test if type pack Us contains the type T.
template<typename T, typename... Us>
constexpr bool Contains = ContainsImpl<T, Us...>::value;

// Test if the tuple contains the type T.
template<typename T, typename... Us>
constexpr bool Contains<T, std::tuple<Us...>> = ContainsImpl<T, Us...>::value;

} // namespace detail

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

  Entity copy() const {
    return _entity.copy();
  }

  Entity copy_to(Container& target) const {
    return _entity.copy_to(target);
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