#pragma once

#include <limits>

namespace secs {

struct TypeIndex {
  template<typename T>
  size_t get() const;

private:
  static const size_t INVALID = std::numeric_limits<size_t>::max();
  mutable size_t _next = 0;

  template<typename T>
  struct Holder {
    static size_t value;
  };
};

template<typename T>
size_t TypeIndex::Holder<T>::value = TypeIndex::INVALID;

template<typename T>
size_t TypeIndex::get() const {
  if (Holder<T>::value == INVALID) {
    Holder<T>::value = _next++;
  }

  return Holder<T>::value;
}

} // namespace secs