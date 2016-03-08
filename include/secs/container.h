#pragma once

#include <cassert>
#include <vector>

#include "secs/component_ops.h"
#include "secs/component_store.h"
#include "secs/lifetime_subscriber.h"
#include "secs/type_index.h"
#include "secs/type_keyed_map.h"

namespace secs {
namespace detail { class Handle; }
template<typename> class ConstComponentView;
template<typename> class MutableComponentView;
class Entity;
template<typename...> class EntityView;

class Container {
public:

  // Create new Entity without any Components.
  Entity create();

  // Create new Entity with default-initialized components of the given types.
  template<typename T, typename... Ts>
  Entity create();

  // Create new Entity with copies or moves of the given Components.
  template<typename T, typename... Ts>
  Entity create(T&&, Ts&&...);

  void destroy(const Entity& entity);

  template<typename... Ts>
  EntityView<Ts...> all();

  size_t size() const {
    return _capacity - _holes.size();
  }

  Entity get(size_t index);

  template<typename T>
  void subscribe(LifetimeSubscriber<T>& subscriber) {
    _event_manager.subscribe<OnCreate<T>>(subscriber);
    _event_manager.subscribe<OnDestroy<T>>(subscriber);
  }

  template<typename T>
  void unsubscribe(LifetimeSubscriber<T>& subscriber) {
    _event_manager.unsubscribe<OnCreate<T>>(subscriber);
    _event_manager.unsubscribe<OnDestroy<T>>(subscriber);
  }

private:

  size_t capacity() const {
    return _capacity;
  }

  uint64_t get_version(size_t index) const {
    return index < _versions.size() ? _versions[index] : 0;
  }

  template<typename T>
  const ComponentStore<T>* find_store() const {
    return _stores.find<ComponentStore<T>>();
  }

  template<typename T>
  ComponentStore<T>* find_store() {
    return _stores.find<ComponentStore<T>>();
  }

  template<typename T>
  const ComponentStore<T>& get_store() const {
    auto store = _stores.find<ComponentStore<T>>();
    assert(store);
    return *store;
  }

  template<typename T>
  ComponentStore<T>& get_store() {
    return _stores.get<ComponentStore<T>>();
  }

  template<typename T>
  bool has_component(size_t index) const {
    auto store = find_store<T>();
    return store && store->contains(index);
  }

  template<typename... Ts>
  bool has_all_components(size_t index) const;

  template<typename T>
  const T* get_component(size_t index) const {
    return get_store<T>().get(index);
  }

  template<typename T>
  T* get_component(size_t index) {
    return get_store<T>().get(index);
  }

  template<typename T, typename... Args>
  void create_component(size_t index, Args&&... args);

  template<typename T>
  void create_components(size_t index);

  template<typename T0, typename T1, typename... T>
  void create_components(size_t index);

  template<typename T, typename... Ts>
  void create_components(size_t, T&&, Ts&&...);
  void create_components(size_t) {}

  template<typename T>
  void destroy_component(size_t index);

  void copy(const Entity& source, const Entity& target);
  void move(const Entity& source, const Entity& target);

  void after_destroy(size_t index);

private:

  template<typename...> struct HasAllComponents;

  size_t                      _capacity = 0;
  std::vector<size_t>         _holes;
  std::vector<uint64_t>       _versions;

  HeterogeneousSet            _stores;
  TypeKeyedMap<ComponentOps>  _ops;
  EventManager                _event_manager;

  template<typename...> friend class EntityView;
  friend class detail::Handle;
  template<typename> friend class ComponentPtr;
  friend class Entity;
};


template<typename... Ts>
bool Container::has_all_components(size_t index) const {
  return HasAllComponents<Ts...>()(*this, index);
}

template<typename T, typename... Ts>
struct Container::HasAllComponents<T, Ts...> {
  bool operator () (const Container& container, size_t index) const {
    return container.has_component<T>(index)
        && HasAllComponents<Ts...>()(container, index);
  }
};

template<>
struct Container::HasAllComponents<> {
  bool operator () (const Container&, size_t) const {
    return true;
  }
};

} // namespace secs

#include "container.inline.h"