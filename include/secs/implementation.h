#pragma once

#include "secs/component_ptr.h"
#include "secs/component_set.h"
#include "secs/container.h"
#include "secs/entity.h"
#include "secs/entity_view.h"
#include "secs/lifetime_events.h"

namespace secs {

inline ContainerEntityView<> Container::entities() {
  return { *this };
}

inline Entity Container::get(size_t index) {
  return Entity(*this, index, get_version(index));
}

template<typename T, typename... Args>
ComponentPtr<T> Container::create_component( const Entity& entity
                                           , Args&&...     args)
{
  auto& s = store<T>();
  assert(!s.contains(entity._index));

  _event_manager.emit(BeforeCreate<T>{ entity });

  _ops.get<T>().template setup<T>();
  s.emplace(entity._index, entity._version, std::forward<Args>(args)...);

  ComponentPtr<T> component(s, entity._index, entity._version);
  _event_manager.emit(AfterCreate<T>{ entity, component });

  return component;
}

template<typename T>
void Container::destroy_component(const Entity& entity) {
  auto& s = store<T>();
  assert(s.contains(entity._index, entity._version));

  ComponentPtr<T> component(s, entity._index, entity._version);
  _event_manager.emit(BeforeDestroy<T>{ entity, component });

  s.erase(entity._index);

  _event_manager.emit(AfterDestroy<T>{ entity });
}


// ComponentOps implementation
template<typename T>
std::enable_if_t<std::is_copy_constructible<T>::value, void>
ComponentOps::copy(const Entity& source, const Entity& target) {
  if (auto sc = source.component<T>().get()) {
    target.create_component<T>(*sc);
  }
}

template<typename T>
std::enable_if_t<!std::is_copy_constructible<T>::value, void>
ComponentOps::copy(const Entity& source, const Entity&) {
  assert(!source.component<T>());
}

template<typename T>
void ComponentOps::destroy(const Entity& entity) {
  if (entity.component<T>()) {
    entity.destroy_component<T>();
  }
}


// Entity implementation
template<typename T>
ComponentPtr<T> Entity::component() const {
  assert(*this);
  return { _container->store<T>(), _index, _version };
}

template<typename T, typename... Args>
ComponentPtr<T> Entity::create_component(Args&&... args) const {
  static_assert(std::is_constructible<T, Args...>::value, "T is not constructible from Args");
  assert(*this);
  return _container->create_component<T>(*this, std::forward<Args>(args)...);
}

template<typename T>
void Entity::destroy_component() const {
  assert(*this);
  _container->destroy_component<T>(*this);
}

template<typename... Ts>
ComponentSet<Ts...> Entity::components() const {
  return components(_container->store_ptrs<Ts...>());
}

template<typename... Ts>
ComponentSet<Ts...>
Entity::components(const std::tuple<ComponentStore<Ts>*...>& stores) const {
  assert(*this);
  return { stores, _index, _version };
}

// ComponentSet implementation
template<typename... Ts>
ComponentSet<Ts...>::ComponentSet(
    const std::tuple<ComponentStore<Ts>*...>& stores
  , size_t                                    index
  , Version                                   version)
  : _stores(stores)
  , _index(index)
  , _version(version)
{}

} // namespace secs
