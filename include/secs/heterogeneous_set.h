#pragma once

#include "secs/any.h"
#include "secs/type_index.h"

namespace secs {

class HeterogeneousSet {
public:

  template<typename T>
  T* find() {
    auto index = _type_index.get<T>();
    return index < _store.size() ? &_store[index].get<T>() : nullptr;
  }

  template<typename T>
  const T* find() const {
    auto index = _type_index.get<T>();
    return index < _store.size() ? &_store[index].get<T>() : nullptr;
  }

  template<typename T>
  T& get() {
    auto index = reserve<T>();
    return _store[index].get<T>();
  }

  template<typename T>
  void set(T&& value) {
    auto index = reserve<T>();
    _store[index] = std::forward<T>(value);
  }

private:

  template<typename T>
  size_t reserve() {
    auto index = _type_index.get<T>();

    if (index >= _store.size()) {
      _store.resize(index + 1);
    }

    return index;
  }

private:
  TypeIndex         _type_index;
  std::vector<Any>  _store;
};

} // namespace secs