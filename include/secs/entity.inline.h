#pragma once

#include <cassert>
#include "secs/entity_store.h"
#include "secs/component_ptr.h"

namespace secs {

template<typename T>
ComponentPtr<T> Entity::component() const {
  assert(valid());
  return { *_store, _index, _version };
}

inline void Entity::destroy() const {
  assert(valid());
  _store->destroy(*this);
}

} // namespace secs