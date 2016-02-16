#pragma once

#include "entity_store.h"
#include "system.h"

namespace secs {

template<typename... Context>
class Environment {
public:

  using System = secs::System<Context...>;

  Entity create_entity() {
    return _entities.create();
  }

  template<typename C, typename... Cs>
  Entity create_entity() {
    auto e = _entities.create();
    e.add_components<C, Cs...>();
    return e;
  }

  template<typename C, typename... Cs>
  Entity create_entity(C&& c, Cs&&... cs) {
    auto e = _entities.create();
    e.add_components(std::forward<C>(c), std::forward<Cs>(cs)...);
    return e;
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

  template<typename T>
  T& add_system(T&& system) {
    static_assert(std::is_convertible<T*, System*>::value, "T must be derived from System");
    _systems.push_back(std::make_unique<T>(std::move(system)));
    return static_cast<T&>(*_systems.back());
  }

  template<typename T, typename... Args>
  T& emplace_system(Args&&... args) {
    static_assert(std::is_convertible<T*, System*>::value, "T must be derived from System");
    _systems.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    return static_cast<T&>(*_systems.back());
  }

  void update(Context... context) {
    for (auto& system : _systems) {
      system->update(*this, std::forward<Context>(context)...);
    }
  }

private:

  EntityStore _entities;
  std::vector<std::unique_ptr<System>> _systems;
};

} // namespace secs