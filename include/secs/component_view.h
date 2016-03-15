#pragma once

// Non-owning subset of Components belonging to the same Entity.

#include "secs/detail.h"

namespace secs {

template<typename> class ComponentStore;

template<typename... Ts>
class ComponentView {
public:
  ComponentView()
    : _index(0)
    , _version(0)
  {}

  explicit operator bool () const {
    return _version > 0 && detail::all(_stores, [=](auto store) {
      return store && store->contains(_index, _version);
    });
  }

  template<typename T>
  T& get() const {
    static_assert( detail::Contains<std::tuple<Ts...>, T>
                 , "This view does not contain T");

    auto store = std::get<ComponentStore<T>*>(_stores);
    assert(_version > 0 && store && store->contains(_index, _version));

    return store->get(_index);
  }

  template<typename T>
  ComponentPtr<T> get_ptr() const {
    static_assert( detail::Contains<std::tuple<Ts...>, T>
                 , "This view does not contain T");

    auto store = std::get<ComponentStore<T>*>(_stores);
    assert(store);

    return { *store, _index, _version };
  }

private:
  ComponentView( const std::tuple<ComponentStore<Ts>*...>& stores
               , size_t                                    index
               , uint64_t                                  version);

private:
  std::tuple<ComponentStore<Ts>*...> _stores;
  size_t                             _index;
  uint64_t                           _version;

  friend class Entity;

  template<typename... Us>
  friend bool operator == ( const ComponentView<Us...>&
                          , const ComponentView<Us...>&);
};

template<typename... Ts>
bool operator == (const ComponentView<Ts...>& a, const ComponentView<Ts...>& b) {
  return a._index   == b._index
      && a._version == b._version
      && a._stores  == b._stores;
}

template<typename... Ts>
bool operator != (const ComponentView<Ts...>& a, const ComponentView<Ts...>& b) {
  return !(a == b);
}

} // namespace secs