#pragma once

#include "secs/entity_store.h"

namespace secs {

class Container {
public:

  Entity create_entity() {
    return _entities.create();
  }

  template<typename C, typename... Cs>
  Entity create_entity() {
    return _entities.create<C, Cs...>();
  }

  template<typename C, typename... Cs>
  Entity create_entity(C&& c, Cs&&... cs) {
    return _entities.create(std::forward<C>(c), std::forward<Cs>(cs)...);
  }

  void destroy_entity(const Entity& entity) {
    _entities.destroy(entity);
  }

  template<typename... Ts>
  EntityView<Ts...> entities() {
    return _entities.filter<Ts...>();
  }

  size_t num_entities() const {
    return _entities.size();
  }

private:

  EntityStore _entities;
};

} // namespace secs