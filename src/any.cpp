#include <utility>
#include "secs/any.h"

using namespace secs;

Any::Any(Any&& other) noexcept
  : _store(other._store)
  , _destroy(other._destroy)
{
  other._store   = nullptr;
  other._destroy = nullptr;
}

Any& Any::operator = (Any&& other) {
  if (this == &other) {
    return *this;
  }

  reset();

  _store = other._store;
  _destroy = other._destroy;

  other._store   = nullptr;
  other._destroy = nullptr;

  return *this;
}

void Any::reset() {
  if (!*this) return;

  _destroy(_store);
  _store   = nullptr;
  _destroy = nullptr;
}
