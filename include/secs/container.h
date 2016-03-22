#pragma once

#include <cassert>
#include <vector>

#include "secs/component_ops.h"
#include "secs/component_store.h"
#include "secs/event_manager.h"
#include "secs/omniset.h"
#include "secs/type_keyed_map.h"
#include "secs/version.h"

namespace secs {
template<typename> class ComponentPtr;
class Entity;
template<typename, typename, typename> class EntityView;
template<typename> class Subscriber;

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

  template<typename E> void subscribe(Subscriber<E>& subscriber) {
    _event_manager.subscribe<E>(subscriber);
  }

  template<typename E> void unsubscribe(Subscriber<E>& subscriber) {
    _event_manager.unsubscribe<E>(subscriber);
  }

  template<typename E> void emit(const E& event) {
    _event_manager.emit(event);
  }

private:
  size_t capacity() const {
    return _capacity;
  }

  Version get_version(size_t index) const {
    return index < _versions.size() ? _versions[index] : Version{};
  }

  bool contains(size_t index) const {
    return index < _versions.size() && _versions[index].exists();
  }

  bool contains(size_t index, Version version) const {
    return index < _versions.size()
        && _versions[index].exists()
        && _versions[index] == version;
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

private:

  size_t                      _capacity = 0;
  std::vector<size_t>         _holes;
  std::vector<Version>        _versions;

  Omniset                     _stores;
  TypeKeyedMap<ComponentOps>  _ops;
  EventManager                _event_manager;

  friend class Entity;
  template<typename, typename, typename> friend class EntityView;
};

} // namespace secs

#include "implementation.h"