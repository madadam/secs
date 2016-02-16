#pragma once

#include <limits>
#include <mutex>
#include <vector>

namespace secs {

// Helper struct that maps types to integer indices. The indices are assigned
// in order, starting from 0, so they are useful for indexing arrays.
struct TypeIndex {
  static const size_t INVALID;

private:
  template<typename T>
  struct Holder {
    static size_t value;
  };


  static size_t _next;
  static std::mutex _mutex;

public:
  template<typename T>
  static size_t get() {
    std::lock_guard<std::mutex> lock(_mutex);

    if (Holder<T>::value == INVALID) {
      Holder<T>::value = _next++;
    }

    return Holder<T>::value;
  }
};

template<typename T>
size_t TypeIndex::Holder<T>::value = TypeIndex::INVALID;

} // namespace secs
