#pragma once

#include <cassert>
#include "handle.h"

namespace secs {

template<typename T>
class ComponentPtr : public Handle<ComponentPtr<T>> {
  using Handle<ComponentPtr<T>>::Handle;
  using Handle<ComponentPtr<T>>::valid;
  using Handle<ComponentPtr<T>>::_store;
  using Handle<ComponentPtr<T>>::_index;

public:

  // Check whether this Ptr belongs to an existing Entity and points to an
  // existing Component.
  explicit operator bool () const {
    return valid() && exists();
  }

  T& operator * () const {
    assert(valid());
    return _store->template get_component<T>(_index);
  }

  T* operator -> () const {
    assert(valid());
    return &_store->template get_component<T>(_index);
  }

  // Creates new Component and assigns it to this Ptr.
  template<typename... Args>
  T& create(Args&&... args) {
    assert(valid());
    return _store->template create_component<T>(_index, std::forward<Args>(args)...);
  }

  // If this Ptr does not currently point to any Component, creates and assigns
  // a new one.
  template<typename... Args>
  T& or_create(Args&&... args) {
    assert(valid());

    if (exists()) {
      return _store->template get_component<T>(_index);
    } else {
      return _store->template create_component<T>(_index, std::forward<Args>(args)...);
    }
  }

  // Destroys the Component pointed to by this Ptr. Invalidates all other Ptrs
  // pointing to the same Component.
  void destroy() {
    assert(valid());
    _store->template destroy_component<T>();

  }

protected:
  bool exists() const {
    return _store->template has_component<T>(_index);
  }

  friend class Entity;
};


} // namespace secs