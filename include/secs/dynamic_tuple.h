#pragma once

#include <tuple>
#include "secs/any.h"
#include "secs/type_indexer.h"

// Tuple with dynamic number of elements. Can contain at most one element per
// type. The types stored in it must be default-constructible.

namespace secs {

class DynamicTuple {
public:
  template<typename T>
  T& get() const {
    auto index = _type_indexer.get<T>();

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
  TypeIndexer              _type_indexer;
  mutable std::vector<Any> _store;
};

} // namespace secs