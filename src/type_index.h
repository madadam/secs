#pragma once

#include <atomic>
#include <limits>
#include <mutex>

namespace detail {
  using std::atomic;
  using std::lock_guard;
  using std::mutex;
  using std::numeric_limits;

  static const size_t INVALID_TYPE_INDEX = numeric_limits<size_t>::max();

  template<typename C>
  struct TypeCounter {
    static atomic<size_t> _value;
    static size_t next() {
      return _value++;
    }
  };

  template<typename C>
  atomic<size_t> TypeCounter<C>::_value{0};

  template<typename T, typename C>
  struct TypeIndex {
  private:
    static atomic<size_t> _value;
    static mutex _mutex;

  public:
    static size_t value() {
      if (_value != INVALID_TYPE_INDEX) {
        return _value;
      }

      lock_guard<mutex> lock(_mutex);

      if (_value != INVALID_TYPE_INDEX) {
        return _value;
      }

      _value = TypeCounter<C>::next();

      return _value;
    }
  };

  template<typename T, typename C> atomic<size_t> TypeIndex<T, C>::_value{INVALID_TYPE_INDEX};
  template<typename T, typename C> mutex TypeIndex<T, C>::_mutex;
}

struct DefaultTypeCategory;

// For each combination of type T and category C, returns a small unique integer.
// This function is safe to call from multiple threads simultaneously.
template<typename T, typename C = DefaultTypeCategory>
size_t type_index() {
  return detail::TypeIndex<T, C>::value();
}
