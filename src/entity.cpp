#include <cassert>
#include "secs/entity.h"

using namespace secs;

Entity::operator bool () const {
  return _container && _container->contains(_index, _version);
}

Entity Entity::copy_to(Container& target) const {
  assert(*this);

  auto result = target.create();
  _container->copy(*this, result);
  return result;
}

void Entity::destroy() const {
  assert(*this);
  _container->destroy(*this);
}

namespace secs {

bool operator == (const Entity& a, const Entity& b) {
  return a._container == b._container
      && a._index     == b._index
      && a._version   == b._version;
}

bool operator < (const Entity& a, const Entity& b) {
  return std::make_tuple(a._container, a._index, a._version)
       < std::make_tuple(b._container, b._index, b._version);
}

} // namespace secs