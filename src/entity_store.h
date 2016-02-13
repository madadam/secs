#pragma once

#include <vector>

#include "component_store.h"
#include "component_view.h"
#include "errors.h"
#include "type_index.h"

namespace secs {

template<typename> class ConstComponentView;
template<typename> class MutableComponentView;
class Entity;
template<typename...> class EntityView;

class EntityStore {
public:

  Entity create();
  void destroy(const Entity& entity);

  Entity get(size_t index);

  template<typename... Ts>
  EntityView<Ts...> filter();

  size_t size() const {
    return _capacity - _holes.size();
  }

private:

  size_t capacity() const {
    return _capacity;
  }

  uint64_t get_version(size_t index) const {
    return index < _versions.size() ? _versions[index] : 0;
  }

  template<typename T>
  bool has_component(size_t index) const {
    return components<T>().contains(index);
  }

  template<typename... Ts>
  bool has_all_components(size_t index) const;

  template<typename T>
  T& get_component(size_t index) {
    auto cs = components<T>();

    if (cs.contains(index)) {
      return cs[index];
    } else {
      throw error::ComponentNotFound();
    }
  }

  template<typename T>
  T& get_or_add_component(size_t index) {
    // TODO: emit on_component_create
    return components<T>()[index];
  }

  template<typename T>
  T& add_component(size_t index, T&& component) {
    // TODO: emit on_component_create
    auto cs = components<T>();

    if (cs.contains(index)) {
      throw error::ComponentAlreadyExists();
    } else {
      return cs[index] = std::forward<T>(component);
    }
  }

  template<typename T, typename... Args>
  T& emplace_component(size_t index, Args&&... args) {
    // TODO: emit on_component_create
    auto cs = components<T>();

    if (cs.contains(index)) {
      throw error::ComponentAlreadyExists();
    } else {
      cs.emplace(index, std::forward<Args>(args)...);
      return cs[index];
    }
  }

  template<typename T>
  void remove_component(size_t index) {
    components<T>().erase(index);
  }

  template<typename T>
  ConstComponentView<const T> components() const {
    auto index = _type_index.get<T>();

    if (index >= _stores.size()) {
      return { _empty_store };
    } else {
      return { _stores[index] };
    }
  }

  template<typename T>
  MutableComponentView<T> components() {
    auto index = _type_index.get<T>();

    if (index >= _stores.size()) {
      _stores.resize(index + 1);
    }

    return { _stores[index] };
  }

private:

  template<typename...> struct HasAllComponents;

  size_t                      _capacity = 0;
  std::vector<size_t>         _holes;
  std::vector<uint64_t>       _versions;

  TypeIndex                   _type_index;
  std::vector<ComponentStore> _stores;
  static ComponentStore       _empty_store;

  template<typename...> friend class EntityView;
  friend class Entity;
};

template<typename... Ts>
bool EntityStore::has_all_components(size_t index) const {
  return HasAllComponents<Ts...>()(*this, index);
}

template<typename T, typename... Ts>
struct EntityStore::HasAllComponents<T, Ts...> {
  bool operator () (const EntityStore& env, size_t index) const {
    return env.has_component<T>(index) && HasAllComponents<Ts...>()(env, index);
  }
};

template<>
struct EntityStore::HasAllComponents<> {
  bool operator () (const EntityStore&, size_t) const {
    return true;
  }
};

} // namespace secs

#include "entity_store.inline.h"