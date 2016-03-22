#pragma once

#include <tuple>
#include "secs/any.h"
#include "secs/type_index.h"

// Container that contains one instance of every default-constructible type
// in the universe.

namespace secs {

class Omniset {
public:
  template<typename T>
  T& get() const {
    auto index = _type_index.get<T>();

    if (index >= _store.size()) {
      _store.resize(index + 1);
    }

    if (!_store[index]) {
      _store[index].template emplace<T>();
    } else {
      assert(_store[index].template contains<T>());
    }

    return _store[index].template get<T>();
  }

  template<typename... Ts>
  std::tuple<Ts&...> slice() const {
    return std::tie(get<Ts>()...);
  }

private:
  TypeIndex                _type_index;
  mutable std::vector<Any> _store;
};

} // namespace secs