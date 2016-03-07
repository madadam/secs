#include <utility>
#include "secs/any.h"

using namespace secs;

Any::Any(const Any& other)
  : _store(copy(other))
  , _destroy(other._destroy)
  , _copy(other._copy)
{}

Any::Any(Any&& other) noexcept
  : _store(other._store)
  , _destroy(other._destroy)
  , _copy(other._copy)
{
  other._store = nullptr;
}

Any& Any::operator = (const Any& other) {
  if (this == &other) {
    return *this;
  }

  reset();

  _store = copy(other);
  _destroy = other._destroy;
  _copy = other._copy;

  return *this;
}

Any& Any::operator = (Any&& other) {
  if (this == &other) {
    return *this;
  }

  reset();

  _store = other._store;
  _destroy = other._destroy;
  _copy = other._copy;

  other._store   = nullptr;
  other._destroy = nullptr;

  return *this;
}

void Any::reset() {
  if (empty()) return;

  _destroy(_store);
  _store   = nullptr;
  _destroy = nullptr;
}

void* Any::copy(const Any& other) {
  if (other.empty()) {
    return nullptr;
  } else {
    assert(other._copy);
    return other._copy(other._store);
  }
}
