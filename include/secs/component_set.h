#pragma once

// Non-owning subset of Components belonging to the same Entity.

#include "secs/detail.h"

namespace secs {
namespace detail {

template<typename T, typename F>
auto only_value(T&& a, F&& f) {
  return f(std::forward<T>(a));
}

template<typename T0, typename T1, typename... Ts, typename F>
auto only_value(T0&& a0, T1&& a1, Ts&&... as, F&& f) {
  auto first = f(std::forward<T0>(a0));
  assert(first == only_value( std::forward<T1>(a1)
                            , std::forward<Ts>(as)...
                            , std::forward<F>(f)));
  return first;
}

}

template<typename> class ComponentStore;

template<typename... Ts>
class ComponentSet {
public:
  ComponentSet()
    : _index(0)
  {}

  explicit ComponentSet(ComponentPtr<Ts>... ptrs)
    : ComponentSet( std::make_tuple(ptrs._store...)
                   , detail::only_value(ptrs..., [](auto p) { return p._index; })
                   , detail::only_value(ptrs..., [](auto p) { return p._version; }))
  {}

  template<typename T>
  std::enable_if_t<detail::Contains<T, Ts...>, bool>
  contains() const {
    assert(_version.exists());
    auto store = std::get<ComponentStore<T>*>(_stores);

    return store->contains(_index, _version);
  }

  template<typename T>
  std::enable_if_t<!detail::Contains<T, Ts...>, void>
  contains() const {
    static_assert(
        detail::Contains<std::tuple<Ts...>, T>
      , "This set does not include T");
  }

  bool contains_all() const {
    return _version.exists() && detail::all(_stores, [=](auto store) {
      return store && store->contains(_index, _version);
    });
  }

  template<typename U0, typename U1, typename... Us>
  bool contains_all() const {
    return contains<U0>() && contains_all<U1, Us...>();
  }

  template<typename U>
  bool contains_all() const {
    return contains<U>();
  }

  template<typename U0, typename U1, typename... Us>
  bool contains_none() const {
    return !contains<U0>() && contains_none<U1, Us...>();
  }

  template<typename U>
  bool contains_none() const {
    return !contains<U>();
  }

  bool contains_none() const {
    return !_version.exists() || detail::all(_stores, [=](auto store) {
      return !store || !store->contains(_index, _version);
    });
  }

  // Retrieve a Component of the given type by reference (must exist).
  template<typename T>
  std::enable_if_t<!std::is_pointer<T>::value &&
                   detail::Contains<T, Ts...>, T&>
  get() const {
    auto store = std::get<ComponentStore<T>*>(_stores);
    assert(_version.exists() && store && store->contains(_index, _version));

    return store->get(_index);
  }

  // Retrieve a Component of the given type by pointer (returns nullptr if it
  // does not exist)
  template<typename T>
  std::enable_if_t<
         std::is_pointer<T>::value
      && detail::Contains<std::remove_pointer_t<T>, Ts...>, T>
  get() const {
    assert(_version.exists());
    auto store = std::get<ComponentStore<std::remove_pointer_t<T>>*>(_stores);

    return store && store->contains(_index, _version) ? &store->get(_index)
                                                      : nullptr;
  }

  // This overload matches if get is called with invalid type. It makes the
  // compiler error more friendly.
  template<typename T>
  std::enable_if_t<
    !detail::Contains<std::remove_pointer_t<T>, Ts...>>
  get() const {
    static_assert(
        detail::Contains<std::remove_pointer_t<T>, Ts...>
      , "This set does not include T");
  }

private:
  ComponentSet( const std::tuple<ComponentStore<Ts>*...>& stores
               , size_t                                   index
               , Version                                  version);

private:
  std::tuple<ComponentStore<Ts>*...> _stores;
  size_t                             _index;
  Version                            _version;

  friend class Entity;

  template<typename... Us>
  friend bool operator == ( const ComponentSet<Us...>&
                          , const ComponentSet<Us...>&);
};


template<>
class ComponentSet<> {
public:

private:
  ComponentSet(const std::tuple<>&, size_t, Version) {}
  friend class Entity;
};

template<typename... Ts>
bool operator == (const ComponentSet<Ts...>& a, const ComponentSet<Ts...>& b) {
  return a._index   == b._index
      && a._version == b._version
      && a._stores  == b._stores;
}

template<typename... Ts>
bool operator != (const ComponentSet<Ts...>& a, const ComponentSet<Ts...>& b) {
  return !(a == b);
}

template<typename... Ts>
ComponentSet<Ts...> make_component_set(ComponentPtr<Ts>... ps) {
  return ComponentSet<Ts...>(ps...);
}

} // namespace secs