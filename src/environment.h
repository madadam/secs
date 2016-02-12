#pragma once

#include <vector>

#include "component_store.h"
#include "component_view.h"
#include "errors.h"
#include "type_index.h"

template<typename> class ConstComponentView;
template<typename> class MutableComponentView;
class Entity;
template<typename...> class EntityView;

// TODO: make sure component destructors are run when environment leaves scope.

class Environment {
public:

  Entity create_entity();

  template<typename... Ts>
  EntityView<Ts...> entities();

  size_t num_entities() const {
    return _entity_capacity - _holes.size();
  }

  size_t entity_capacity() const {
    return _entity_capacity;
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

private:

  size_t entities_capacity() const {
    return _entity_capacity;
  }

  template<typename T>
  ConstComponentView<const T> components() const {
    auto index = type_index<T>();

    if (index >= _stores.size()) {
      return { _empty_store };
    } else {
      return { _stores[index] };
    }
  }

  template<typename T>
  MutableComponentView<T> components() {
    auto index = type_index<T>();

    if (index >= _stores.size()) {
      _stores.resize(index + 1);
    }

    return { _stores[index] };
  }

private:

  size_t                      _entity_capacity = 0;
  std::vector<size_t>         _holes;

  std::vector<ComponentStore> _stores;
  static ComponentStore       _empty_store;

  template<typename...> friend class EntityView;
};

#include "environment.inline.h"