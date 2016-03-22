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
class ComponentView {
public:
  ComponentView()
    : _index(0)
    , _version(0)
  {}

  explicit ComponentView(ComponentPtr<Ts>... ptrs)
    : ComponentView( std::make_tuple(ptrs._store...)
                   , detail::only_value(ptrs..., [](auto p) { return p._index; })
                   , detail::only_value(ptrs..., [](auto p) { return p._version; }))
  {}

  explicit operator bool () const {
    return _version.exists() && detail::all(_stores, [=](auto store) {
      return store && store->contains(_index, _version);
    });
  }

  template<typename T>
  T& get() const {
    static_assert( detail::Contains<std::tuple<Ts...>, T>
                 , "This view does not contain T");

    auto store = std::get<ComponentStore<T>*>(_stores);
    assert(_version.exists() && store && store->contains(_index, _version));

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
               , Version                                   version);

private:
  std::tuple<ComponentStore<Ts>*...> _stores;
  size_t                             _index;
  Version                            _version;

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

template<typename... Ts>
ComponentView<Ts...> make_component_view(ComponentPtr<Ts>... ps) {
  return ComponentView<Ts...>(ps...);
}

} // namespace secs