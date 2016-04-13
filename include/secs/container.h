#pragma once

#include <cassert>
#include <vector>

#include "secs/component_ops.h"
#include "secs/component_store.h"
#include "secs/event_traits.h"
#include "secs/omniset.h"
#include "secs/signal.h"
#include "secs/type_keyed_map.h"
#include "secs/version.h"

namespace secs {
template<typename> class ComponentPtr;
class Entity;
template<typename, typename...> class EntityFilter;
class EntityView;
template<typename> class Receiver;

class Container {
public:
  ~Container();

  // Create new Entity.
  Entity create();

  // Destroy Entity.
  void destroy(const Entity& entity);

  // Get collection of all Entities in this Container.
  template<typename... Ts>
  EntityFilter<EntityView, Ts...> entities();

  size_t size() const {
    return _capacity - _holes.size();
  }

  Entity get(size_t index);

  // Connect handler to be called when Event of type E is emitted.
  template<typename E, typename F>
  auto connect(F&& f) {
    return _signals.get<Signal<E>>().connect(std::forward<F>(f));
  }

  template<typename E, typename T>
  std::enable_if_t<CanHandleEvent<T, E>, Connection>
  connect();

  template<typename E>
  void emit(const E& event) const {
    _signals.get<Signal<E>>()(event);
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

  Omniset                     _signals;

  friend class Entity;
  template<typename, typename...> friend class EntityFilter;
  friend class EntityView;
};

} // namespace secs

#include "implementation.h"