#pragma once

#include "entity.h"
#include "entity_view.h"

namespace detail {

template<typename... Ts> struct HasAllComponents;

template<typename T, typename... Ts>
struct HasAllComponents<T, Ts...> {
  bool operator () (const Environment& env, size_t index) const {
    return env.has_component<T>(index) && HasAllComponents<Ts...>()(env, index);
  }
};

template<>
struct HasAllComponents<> {
  bool operator () (const Environment&, size_t) const {
    return true;
  }
};

} // namespace detail

template<typename... Ts>
EntityView<Ts...> Environment::entities() {
  return { *this };
}

template<typename... Ts>
bool Environment::has_all_components(size_t index) const {
  return detail::HasAllComponents<Ts...>()(*this, index);
}
