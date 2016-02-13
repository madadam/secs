#pragma once

#include "entity.h"
#include "entity_view.h"

namespace secs {

inline Entity EntityStore::get(size_t index) {
  return Entity(*this, index, get_version(index));
}

template<typename... Ts>
EntityView<Ts...> EntityStore::filter() {
  return { *this };
}

} // namespace secs