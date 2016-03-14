#pragma once

#include <cassert>
#include <utility>
#include "secs/component_store.h"

namespace secs {

template<typename T>
class ComponentPtr {
public:
  ComponentPtr()
    : _store(nullptr)
    , _index(0)
    , _version(0)
  {}

  ComponentPtr(std::nullptr_t) : ComponentPtr()
  {}

  explicit operator bool () const {
    return _store && _version > 0 && _store->contains(_index, _version);
  }

  T& operator * () const {
    assert(*this);
    return _store->get(_index);
  }

  T* operator -> () const {
    assert(*this);
    return &_store->get(_index);
  }

  T* get() const {
    return *this ? &_store->get(_index) : nullptr;
  }

private:
  ComponentPtr(ComponentStore<T>& store, size_t index, uint64_t version)
    : _store(&store)
    , _index(index)
    , _version(version)
  {}

private:
  ComponentStore<T>* _store;
  size_t             _index;
  uint64_t           _version;

  template<typename U>
  friend bool operator == (const ComponentPtr<U>&, const ComponentPtr<U>&);

  template<typename U>
  friend bool operator < (const ComponentPtr<U>&, const ComponentPtr<U>&);

  friend class Container;
  friend class Entity;
};

template<typename T>
bool operator == (const ComponentPtr<T>& a, const ComponentPtr<T>& b) {
  return a._store == b._store
      && a._index == b._index
      && a._version == b._version;
}

template<typename T>
bool operator != (const ComponentPtr<T>& a, const ComponentPtr<T>& b) {
  return !(a == b);
}

template<typename T>
bool operator < (const ComponentPtr<T>& a, const ComponentPtr<T>& b) {
  return std::make_tuple(a._store, a._index, a._version)
       < std::make_tuple(b._store, b._index, b._version);
}

} // namespace secs