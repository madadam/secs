#pragma once

#include <algorithm>
#include <vector>

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
//
// TODO: optimize this
struct TypeIndexer {
private:
  typedef void (*Key)();

  template<typename T>
  struct KeyGen {
    static void dummy() {}
    static constexpr Key value() {
      return &dummy;
    }
  };

  struct KeyComparer {
    bool operator () ( const std::pair<Key, size_t>& a
                     , const std::pair<Key, size_t>& b) const
    {
      return a.first < b.first;
    }

    bool operator () (const std::pair<Key, size_t>& a, Key b) const {
      return a.first < b;
    }

    bool operator () (Key a, const std::pair<Key, size_t>& b) const {
      return a < b.first;
    }
  };


  // Sorted vector so we can look stuff up using binary search
  // (lower_bound/upper_bound algorithms)
  mutable std::vector<std::pair<Key, size_t>> _map;
  mutable size_t _next = 0;

public:

  template<typename T>
  size_t get() const {
    auto key = KeyGen<T>::value();
    auto it = std::lower_bound(_map.begin(), _map.end(), key, KeyComparer());

    if (it == _map.end() || it->first != key) {
      it = _map.insert( std::upper_bound( _map.begin()
                                        , _map.end()
                                        , key
                                        , KeyComparer())
                      , { key, _next++ });
    }

    return it->second;
  }
};

} // namespace secs
