#pragma once

#include <boost/container/flat_map.hpp>

namespace secs {

//
// Helper struct that maps types to integer indices. The indices are assigned
// in order, starting from 0, so they are useful for indexing arrays.
//
// Examples:
//   TypeIndex ti;
//   assert(ti.get<Foo>() == ti.get<Foo>());
//   assert(ti.get<Bar>() != ti.get<Foo>());
//
struct TypeIndex {
private:
  typedef void (*Key)();

  template<typename T>
  struct KeyGen {
    static void dummy() {}
    static constexpr Key value() {
      return &dummy;
    }
  };

  mutable boost::container::flat_map<Key, size_t> _map;
  mutable size_t _next = 0;

public:

  template<typename T>
  size_t get() const {
    auto key = KeyGen<T>::value();
    auto it = _map.find(key);

    if (it == _map.end()) {
      it = _map.insert(it, std::make_pair(key, _next++));
    }

    return it->second;
  }
};

} // namespace secs
