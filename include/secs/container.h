#pragma once

#include <cassert>
#include <vector>

#include "secs/component_ops.h"
#include "secs/component_store.h"
#include "secs/detail.h"
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
  std::enable_if_t<detail::IsCallable<F, E&>, Connection<const E&>>
  connect(F&& f) {
    return _signals.get<Signal<const E&>>().connect(std::forward<F>(f));
  }

  template<typename E>
  void emit(const E& event) const {
    _signals.get<Signal<const E&>>()(event);
  }

private:
  // Entity metadata.
  struct Meta {
    Version version;
    Omniset signals;
  };

  size_t capacity() const {
    return _capacity;
  }

  Version get_version(size_t index) const {
    return index < _meta.size() ? _meta[index].version : Version{};
  }

  bool contains(size_t index) const {
    return index < _meta.size() && _meta[index].version.exists();
  }

  bool contains(size_t index, Version version) const {
    return index < _meta.size()
        && _meta[index].version.exists()
        && _meta[index].version == version;
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

  template<typename E, typename F>
  std::enable_if_t<detail::IsCallable<F, E&>, Connection<const E&>>
  connect(const Entity&, F&&);

  template<typename E>
  void emit(const Entity&, const E&) const;

private:

  size_t                      _capacity = 0;
  std::vector<size_t>         _holes;
  std::vector<Meta>           _meta;

  Omniset                     _stores;
  Omniset                     _signals;
  TypeKeyedMap<ComponentOps>  _ops;

  friend class Entity;
  template<typename, typename...> friend class EntityFilter;
  friend class EntityView;
};

} // namespace secs

#include "implementation.h"