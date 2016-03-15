#pragma once

#include "secs/component_ptr.h"
#include "secs/component_view.h"
#include "secs/container.h"
#include "secs/entity.h"
#include "secs/entity_view.h"
#include "secs/lifetime_subscriber.h"

namespace secs {

// Container implementation
template<typename T, typename... Args>
struct InvokeOnCreate {
  void operator () (const Entity&, const ComponentPtr<T>&) const;
};

template<typename T>
struct InvokeOnCreate<T, const T&> {
  void operator () (const Entity&, const ComponentPtr<T>&) const;
};

template<typename T>
struct InvokeOnCreate<T, T&> {
  void operator () (const Entity&, const ComponentPtr<T>&) const;
};

template<typename T>
struct InvokeOnCreate<T, T&&> {
  void operator () (const Entity&, const ComponentPtr<T>&) const;
};

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

  InvokeOnCreate<T, decltype(args)...>()(entity, component);

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

template<typename T0, typename T1, typename... Ts>
void Container::prioritize() const {
  prioritize<T0>();
  prioritize<T1, Ts...>();
}

template<typename T>
void Container::prioritize() const {
  _ops.find<T>();
}

// ComponentOps implementation
template<typename T>
typename std::enable_if<std::is_copy_constructible<T>::value, void>::type
ComponentOps::copy(const Entity& source, const Entity& target) {
  target.create_component<T>(*source.component<T>());
}

template<typename T>
typename std::enable_if<!std::is_copy_constructible<T>::value, void>::type
ComponentOps::copy(const Entity&, const Entity&) {
  assert(false);
}

template<typename T>
void ComponentOps::move(const Entity& source, const Entity& target) {
  target.create_component<T>(std::move(*source.component<T>()));
  source.destroy_component<T>();
}

template<typename T>
void ComponentOps::destroy(const Entity& entity) {
  entity.destroy_component<T>();
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

template<typename T, typename... Args>
void secs::InvokeOnCreate<T, Args...>::operator () (
  const secs::Entity& entity, const secs::ComponentPtr<T>& component
) const
{
  using secs::on_create;
  on_create(entity, component);
}

template<typename T>
void secs::InvokeOnCreate<T, const T&>::operator () (
  const secs::Entity& entity, const secs::ComponentPtr<T>& component
) const
{
  using secs::on_create;
  on_copy(entity, component);
}

template<typename T>
void secs::InvokeOnCreate<T, T&>::operator () (
  const secs::Entity& entity, const secs::ComponentPtr<T>& component
) const
{
  using secs::on_create;
  on_copy(entity, component);
}

template<typename T>
void secs::InvokeOnCreate<T, T&&>::operator () (
  const secs::Entity& entity, const secs::ComponentPtr<T>& component
) const
{
  using secs::on_create;
  on_move(entity, component);
}

template<typename T>
void secs::invoke_on_destroy( const secs::Entity&          entity
                            , const secs::ComponentPtr<T>& component)
{
  using secs::on_destroy;
  on_destroy(entity, component);
}

// Default lifetime callbacks.

template<typename T>
void secs::on_create(const Entity&, const ComponentPtr<T>&) {}

template<typename T>
void secs::on_copy(const Entity& entity, const ComponentPtr<T>& component) {
  using secs::on_create;
  on_create(entity, component);
}

template<typename T>
void secs::on_move(const Entity& entity, const ComponentPtr<T>& component) {
  using secs::on_create;
  on_create(entity, component);
}

template<typename T>
void secs::on_destroy(const Entity&, const ComponentPtr<T>&) {}
