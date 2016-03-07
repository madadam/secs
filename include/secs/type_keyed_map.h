#pragma once

#include <vector>
#include "secs/type_index.h"

// Map-like container where keys are types.

namespace secs {

template<typename V>
class TypeKeyedMap {
  static_assert(std::is_default_constructible<V>::value, "value type must be default constructible");

public:

  using iterator       = typename std::vector<V>::iterator;
  using const_iterator = typename std::vector<V>::const_iterator;

public:

  iterator begin() { return _values.begin(); }
  iterator end()   { return _values.end(); }

  const_iterator begin() const { return _values.begin(); }
  const_iterator end()   const { return _values.end(); }

  template<typename T>
  V* find() {
    auto index = _type_index.get<T>();
    return index < _values.size() ? &_values[index] : nullptr;
  }

  template<typename T>
  const V* find() const {
    auto index = _type_index.get<T>();
    return index < _values.size() ? &_values[index] : nullptr;
  }

  template<typename T>
  V& get() {
    auto index = _type_index.get<T>();
    reserve(index);
    return _values[index];
  }

  template<typename T>
  void set(const V& value) {
    reserve(index);
    _values[index] = value;
  }

  template<typename T>
  void set(V&& value) {
    reserve(index);
    _values[index] = std::move(value);
  }

private:

  void reserve(size_t index) {
    if (index >= _values.size()) {
      _values.resize(index + 1);
    }
  }

private:
  TypeIndex       _type_index;
  std::vector<V>  _values;
};

} // namespace secs