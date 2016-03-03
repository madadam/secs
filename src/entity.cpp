#include "secs/entity.h"

using namespace secs;

Entity Entity::copy() {
  return copy_to(*_container);
}

Entity Entity::copy_to(Container& target) {
  assert(valid());

  auto result = target.create();
  _container->copy(_index, target, result._index);
  return result;
}

Entity Entity::move_to(Container& target) {
  assert(valid());

  // Moving within the same container has no observable effect.
  if (_container == &target) return *this;

  auto result = target.create();
  _container->move(_index, target, result._index);
  _container->destroy(*this);

  return result;
}

void Entity::destroy() const {
  assert(valid());
  _container->destroy(*this);
}
