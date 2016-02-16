#pragma once

#include "secs/handle.h"

namespace secs {

template<typename> class ComponentPtr;
class EntityStore;

class Entity : public Handle<Entity> {
  using Handle::Handle;

public:
  explicit operator bool () const {
    return valid();
  }

  template<typename T> ComponentPtr<T> component() const;
  template<typename T0, typename T1, typename... Ts> void add_components() const;
  template<typename T> void add_components() const;
  template<typename T, typename... Ts> void add_components(T&, Ts&&...) const;

  void destroy() const;

private:
  void add_components() const {};
  friend class EntityStore;
};

template<typename T0, typename T1, typename... Ts> void Entity::add_components() const {
  component<T0>().create();
  add_components<T1, Ts...>();
}

template<typename T> void Entity::add_components() const {
  component<T>().create();
}

template<typename T, typename... Ts> void Entity::add_components(T& c, Ts&&... cs) const {
  component<T>().create(std::forward<T>(c));
  add_components(std::forward<Ts>(cs)...);
}

} // namespace secs

#include "entity.inline.h"