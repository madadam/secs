#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include "secs/version.h"

namespace secs {

template<typename> class ComponentPtr;
class Container;
template<typename...> class FilteredEntity;

class Entity {
public:
  Entity()
    : _container(nullptr)
    , _index(0)
  {}

  template<typename... Ts>
  Entity(const FilteredEntity<Ts...>& other);

  explicit operator bool () const noexcept;

  template<typename T> ComponentPtr<T> component() const;

  // Create a Component of the given type using the given arguments for the
  // Component constructor.
  // If a component of the same type already exists, it will be replaced.
  template<typename T, typename... Args>
  std::enable_if_t<std::is_constructible<T, Args...>::value, ComponentPtr<T>>
  create_component(Args&&... args) const;

  // Creates a Component of the given type unless one already exists.
  template<typename T> ComponentPtr<T> ensure_component() const {
    if (auto p = component<T>()) {
      return p;
    } else {
      return create_component<T>();
    }
  }

  // Destroys the Component of the given type. Do nothing if it doesn't exist.
  template<typename T> void destroy_component() const;

  Container& container() const {
    return *_container;
  }

  Entity copy() const {
    return copy_to(*_container);
  }

  Entity copy_to(Container&) const;
  void destroy() const;

private:
  Entity(Container& container, size_t index, Version version)
    : _container(&container)
    , _index(index)
    , _version(version)
  {}

private:
  Container* _container;
  size_t     _index;
  Version    _version;

  friend class Container;
  template<typename, typename...> friend class EntityFilter;
  template<typename...> friend class FilteredEntity;

  friend bool operator == (const Entity&, const Entity&);
  friend bool operator < (const Entity&, const Entity&);
};

inline bool operator != (const Entity& a, const Entity& b) { return !(a == b); }

} // namespace secs
