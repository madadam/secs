#pragma once

#include "secs/entity.h"
#include "secs/entity_view.h"

namespace secs {

template<typename C, typename... Cs>
Entity EntityStore::create() {
  auto e = create();
  e.add_components<C, Cs...>();
  return e;
}

template<typename C, typename... Cs>
Entity EntityStore::create(C&& c, Cs&&... cs) {
  auto e = create();
  e.add_components(std::forward<C>(c), std::forward<Cs>(cs)...);
  return e;
}

inline Entity EntityStore::get(size_t index) {
  return Entity(*this, index, get_version(index));
}

template<typename... Ts>
EntityView<Ts...> EntityStore::filter() {
  return { *this };
}

} // namespace secs