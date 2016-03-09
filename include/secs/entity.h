#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

namespace secs {

template<typename> class ComponentPtr;
class Container;

class Entity {
public:
  Entity(std::nullptr_t)
    : _container(nullptr)
    , _index(0)
    , _version(0)
  {}

  Entity& operator = (std::nullptr_t);

  explicit operator bool () const;

  template<typename T> ComponentPtr<T> component() const;

  // Create a Component of the given type using the given arguments for the
  // Component constructor.
  template<typename T, typename... Args>
  ComponentPtr<T> create_component(Args&&... args) const;

  // Create a Component of the given type unless one already exists.
  template<typename T> ComponentPtr<T> ensure_component() const {
    if (auto p = component<T>()) {
      return p;
    } else {
      return create_component<T>();
    }
  }

  // Destroys the Component of the given type.
  template<typename T> void destroy_component() const;

  Container& container() const {
    return *_container;
  }

  Entity copy();
  Entity copy_to(Container&);
  Entity move_to(Container&);
  void destroy() const;

private:
  Entity(Container& container, size_t index, uint64_t version)
    : _container(&container)
    , _index(index)
    , _version(version)
  {}

private:
  Container* _container;
  size_t     _index;
  uint64_t   _version;

  friend class Container;
  friend bool operator == (const Entity&, const Entity&);
  friend bool operator < (const Entity&, const Entity&);
};

inline bool operator == (const Entity& e, std::nullptr_t) { return !(bool) e; }
inline bool operator != (const Entity& e, std::nullptr_t) { return  (bool) e; }

inline bool operator != (const Entity& a, const Entity& b) { return !(a == b); }

} // namespace secs

#include "container.inline.h"