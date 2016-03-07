#include <cassert>
#include "secs/entity.h"

using namespace secs;

Entity Entity::copy() {
  return copy_to(*_container);
}

Entity Entity::copy_to(Container& target) {
  assert(valid());

  auto result = target.create();
  _container->copy(*this, result);
  return result;
}

Entity Entity::move_to(Container& target) {
  assert(valid());

  // Moving within the same container has no observable effect.
  if (_container == &target) return *this;

  auto result = target.create();
  _container->move(*this, result);
  return result;
}

void Entity::destroy() const {
  assert(valid());
  _container->destroy(*this);
}
