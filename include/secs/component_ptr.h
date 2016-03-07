#pragma once

#include <cassert>
#include <utility>
#include "secs/container.h"
#include "secs/handle.h"

namespace secs {

class Entity;

template<typename T>
class ComponentPtr : public Handle<ComponentPtr<T>> {
  using Handle<ComponentPtr<T>>::Handle;
  using Handle<ComponentPtr<T>>::valid;
  using Handle<ComponentPtr<T>>::_container;
  using Handle<ComponentPtr<T>>::_index;

public:

  // Check whether this Ptr belongs to an existing Entity and points to an
  // existing Component.
  explicit operator bool () const {
    return valid() && exists();
  }

  T& operator * () const {
    assert(valid() && exists());
    return *_container->template get_component<T>(_index);
  }

  T* operator -> () const {
    assert(valid() && exists());
    return _container->template get_component<T>(_index);
  }

  T* get() const {
    return valid() ? _container->template get_component<T>(_index) : nullptr;
  }

  // Creates new Component and assigns it to this Ptr.
  template<typename... Args>
  ComponentPtr create(Args&&... args) {
    static_assert(std::is_constructible<T, Args...>::value, "component is not constructible using the given args");
    assert(valid());
    _container->template create_component<T>(_index, std::forward<Args>(args)...);
    return *this;
  }

  // If this Ptr does not currently point to any Component, creates and assigns
  // a new one.
  template<typename... Args>
  ComponentPtr or_create(Args&&... args) {
    static_assert(std::is_constructible<T, Args...>::value, "component is not constructible using the given args");
    assert(valid());

    if (!exists()) {
      _container->template create_component<T>(_index, std::forward<Args>(args)...);
    }

    return *this;
  }

  // Access the Entity that owns this Component.
  Entity entity() const;

  // Access other Components on the same Entity that owns this Component.
  template<typename U>
  ComponentPtr<U> component() const {
    return entity().component<U>();
  }

  // Destroys this Component. Invalidates all other Ptrs pointing to the same
  // Component.
  void destroy() {
    assert(valid());
    _container->template destroy_component<T>(_index);
  }

protected:
  bool exists() const {
    return _container->template has_component<T>(_index);
  }

  friend class Entity;
};


} // namespace secs