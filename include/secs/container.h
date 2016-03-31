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
class ContainerEntityView;
class Entity;
template<typename, typename...> class LoadEntityView;
template<typename> class Subscriber;

class Container {
public:
  ~Container();

  // Create new Entity.
  Entity create();

  // Destroy Entity.
  void destroy(const Entity& entity);

  // Get collection of all Entities in this Container. This collection can be
  // further refined using need(), skip() and load().
  LoadEntityView<ContainerEntityView> entities();

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

  template<typename... Ts>
  std::tuple<ComponentStore<Ts>*...> store_ptrs() const {
    return std::make_tuple(&_stores.get<ComponentStore<Ts>>()...);
  }

  template<typename T>
  bool contains_store(const ComponentStore<T>& other_store) const {
    return &store<T>() == &other_store;
  }

  template<typename T, typename... Args>
  ComponentPtr<T> create_component(const Entity&, Args&&... args);

  template<typename T>
  void destroy_component(const Entity&);

  template<typename T>
  ComponentPtr<T> create_component_unless_exists();

  template<typename T>
  void destroy_component_if_exists();

  void copy(const Entity& source, const Entity& target);

private:

  size_t                      _capacity = 0;
  std::vector<size_t>         _holes;
  std::vector<Version>        _versions;

  EventManager                _event_manager;
  Omniset                     _stores;
  TypeKeyedMap<ComponentOps>  _ops;

  friend class ContainerEntityView;
  friend class Entity;
  template<typename...> friend class LoadedEntity;
  template<typename, typename...> friend class LoadEntityView;
};

} // namespace secs

#include "implementation.h"