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
template<typename, typename, typename> class EntityView;
template<typename> class LifetimeSubscriber;

class Container {
public:

  // Create new Entity.
  Entity create();
  // Destroy Entity.
  void destroy(const Entity& entity);

  // Get collection of all Entities in this Container. This collection can be
  // further refined using need(), skip() and load().
  EntityView< std::tuple<>
            , std::tuple<>
            , std::tuple<>> entities();

  size_t size() const {
    return _capacity - _holes.size();
  }

  Entity get(size_t index);

  template<typename T> void subscribe  (LifetimeSubscriber<T>& subscriber);
  template<typename T> void unsubscribe(LifetimeSubscriber<T>& subscriber);

  // Specify in what order Component types are processed.
  template<typename T0, typename T1, typename... Ts> void prioritize() const;
  template<typename T> void prioritize() const;

private:
  struct Meta {
    bool     exists  = false;
    uint64_t version = 0;

    uint64_t create() {
      exists = true;
      return ++version;
    }

    uint64_t destroy() {
      exists = false;
      return ++version;
    }
  };

  size_t capacity() const {
    return _capacity;
  }

  uint64_t get_version(size_t index) const {
    return index < _meta.size() ? _meta[index].version : 0;
  }

  bool contains(size_t index) const {
    return index < _meta.size() && _meta[index].exists;
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

  template<typename T>
  void emit_on_create(const Entity& entity, const ComponentPtr<T>&) const;

  void cleanup(size_t index);

private:

  size_t                      _capacity = 0;
  std::vector<size_t>         _holes;
  std::vector<Meta>           _meta;

  Omniset                     _stores;
  TypeKeyedMap<ComponentOps>  _ops;
  EventManager                _event_manager;

  friend class Entity;
  template<typename, typename, typename> friend class EntityView;
};

template<typename, typename...> struct InvokeOnCreate;
template<typename T> void invoke_on_destroy(const Entity&, const ComponentPtr<T>&);

template<typename T> void on_create (const Entity&, const ComponentPtr<T>&);
template<typename T> void on_copy   (const Entity&, const ComponentPtr<T>&);
template<typename T> void on_move   (const Entity&, const ComponentPtr<T>&);
template<typename T> void on_destroy(const Entity&, const ComponentPtr<T>&);

} // namespace secs

#include "implementation.h"