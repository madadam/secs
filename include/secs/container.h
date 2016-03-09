#pragma once

#include <cassert>
#include <vector>

#include "secs/component_ops.h"
#include "secs/component_store.h"
#include "secs/event.h"
#include "secs/omniset.h"
#include "secs/type_keyed_map.h"

namespace secs {
template<typename> class ComponentPtr;
class Entity;
template<typename...> class EntityView;
template<typename> class LifetimeSubscriber;

class Container {
public:

  // Create new Entity.
  Entity create();
  // Destroy Entity.
  void destroy(const Entity& entity);

  template<typename... Ts, typename F>
  void each(F&&);

  size_t size() const {
    return _capacity - _holes.size();
  }

  Entity get(size_t index);

  template<typename T> void subscribe  (LifetimeSubscriber<T>& subscriber);
  template<typename T> void unsubscribe(LifetimeSubscriber<T>& subscriber);

private:

  size_t capacity() const {
    return _capacity;
  }

  uint64_t get_version(size_t index) const {
    return index < _versions.size() ? _versions[index] : 0;
  }

  template<typename T>
  ComponentStore<T>& store() const {
    return _stores.get<ComponentStore<T>>();
  }

  template<typename T, typename... Args>
  ComponentPtr<T> create_component(const Entity&, Args&&... args);

  template<typename T>
  void destroy_component(const Entity&);

  void copy(const Entity& source, const Entity& target);
  void move(const Entity& source, const Entity& target);

  void after_destroy(size_t index);

private:

  template<typename...> struct HasAllComponents;

  size_t                      _capacity = 0;
  std::vector<size_t>         _holes;
  std::vector<uint64_t>       _versions;

  Omniset                     _stores;
  TypeKeyedMap<ComponentOps>  _ops;
  EventManager                _event_manager;

  template<typename...> friend class EntityView;
  template<typename> friend class ComponentPtr;
  friend class Entity;
};

} // namespace secs

#include "container.inline.h"