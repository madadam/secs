#pragma once

#include "secs/component_ptr.h"
#include "secs/component_view.h"
#include "secs/container.h"
#include "secs/entity.h"
#include "secs/entity_view.h"
#include "secs/lifetime_subscriber.h"

// DEBUG
#include <iostream>

namespace secs {

inline EntityView<std::tuple<>, std::tuple<>, std::tuple<>>
Container::entities() {
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

  _ops.get<T>().template setup<T>();
  s.emplace(entity._index, entity._version, std::forward<Args>(args)...);

  ComponentPtr<T> component(s, entity._index, entity._version);

  invoke_on_create(entity, component);

  secs::OnCreate<T> event{ entity, component };
  _event_manager.emit(event);

  return component;
}

template<typename T>
void Container::destroy_component(const Entity& entity) {
  auto& s = store<T>();
  assert(s.contains(entity._index, entity._version));

  ComponentPtr<T> component(s, entity._index, entity._version);

  invoke_on_destroy(entity, component);

  OnDestroy<T> event{ entity, component };
  _event_manager.emit(event);

  s.erase(entity._index);
}

template<typename T>
void Container::subscribe(LifetimeSubscriber<T>& subscriber) {
  _event_manager.subscribe<OnCreate<T>>(subscriber);
  _event_manager.subscribe<OnDestroy<T>>(subscriber);
}

template<typename T>
void Container::unsubscribe(LifetimeSubscriber<T>& subscriber) {
  _event_manager.unsubscribe<OnCreate<T>>(subscriber);
  _event_manager.unsubscribe<OnDestroy<T>>(subscriber);
}

// ComponentOps implementation
template<typename T>
typename std::enable_if<std::is_copy_constructible<T>::value, void>::type
ComponentOps::copy(const Entity& source, const Entity& target) {
  if (source.component<T>()) {
    target.copy_component_from<T>(source);
  }
}

template<typename T>
typename std::enable_if<!std::is_copy_constructible<T>::value, void>::type
ComponentOps::copy(const Entity& source, const Entity&) {
  assert(!source.component<T>());
}

template<typename T>
void ComponentOps::move(const Entity& source, const Entity& target) {
  if (source.component<T>()) {
    target.move_component_from<T>(source);
  }
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
ComponentPtr<T> Entity::copy_component_from(const Entity& source) const {
  auto result = create_component<T>(*source.component<T>());
  invoke_on_copy(source, *this, result);
  return result;
}

template<typename T>
ComponentPtr<T> Entity::move_component_from(const Entity& source) const {
  auto result = create_component<T>(std::move(*source.component<T>()));
  source.destroy_component<T>();
  invoke_on_move(source, *this, result);
  return result;
}

template<typename T>
void Entity::destroy_component() const {
  assert(*this);
  _container->destroy_component<T>(*this);
}

template<typename... Ts>
ComponentView<Ts...> Entity::components() const {
  assert(*this);
  return { std::make_tuple(&_container->store<Ts>()...), _index, _version };
}

// ComponentView implementation
template<typename... Ts>
ComponentView<Ts...>::ComponentView(
    const std::tuple<ComponentStore<Ts>*...>& stores
  , size_t                                    index
  , uint64_t                                  version)
  : _stores(stores)
  , _index(index)
  , _version(version)
{}

} // namespace secs

// This needs to be defined outside of any namespace, as it exploits Argument
// Dependent Lookup.

template<typename T>
void secs::invoke_on_create( const secs::Entity&          entity
                           , const secs::ComponentPtr<T>& component)
{
  using secs::on_create;
  on_create(entity, component);
}

template<typename T>
void secs::invoke_on_destroy( const secs::Entity&          entity
                            , const secs::ComponentPtr<T>& component)
{
  using secs::on_destroy;
  on_destroy(entity, component);
}

template<typename T>
void secs::invoke_on_copy( const secs::Entity&          source
                         , const secs::Entity&          target
                         , const secs::ComponentPtr<T>& component)
{
  using secs::on_copy;
  on_copy(source, target, component);
}

template<typename T>
void secs::invoke_on_move( const secs::Entity&          source
                         , const secs::Entity&          target
                         , const secs::ComponentPtr<T>& component)
{
  using secs::on_move;
  on_move(source, target, component);
}

// Default lifetime callbacks.

template<typename T>
void secs::on_create(const Entity&, const ComponentPtr<T>&) {}

template<typename T>
void secs::on_copy(const Entity&, const Entity&, const ComponentPtr<T>&) {}

template<typename T>
void secs::on_move(const Entity&, const Entity&, const ComponentPtr<T>&) {}

template<typename T>
void secs::on_destroy(const Entity&, const ComponentPtr<T>&) {}
