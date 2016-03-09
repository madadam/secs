#pragma once

#include "secs/component_ptr.h"
#include "secs/container.h"
#include "secs/entity.h"
#include "secs/lifetime_subscriber.h"
#include "secs/misc.h"

namespace secs {

// Container implementation
template<typename... Ts, typename F>
void Container::each(F&& f) {
  static_assert(is_callable<F, Entity, Ts&...>, "f is not callable with (Entity, Ts&...)");

  auto ss = _stores.slice<ComponentStore<Ts>...>();

  for (size_t i = 0; i < _capacity; ++i) {
    if (all(ss, [i](auto& store) { return store.contains(i); })) {
      f(get(i), std::get<ComponentStore<Ts>&>(ss).get(i)...);
    }
  }
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

  OnCreate<T> event{ entity, component };
  _event_manager.emit(event);

  return component;
}

template<typename T>
void Container::destroy_component(const Entity& entity) {
  auto& s = store<T>();
  assert(s.contains(entity._index, entity._version));

  OnDestroy<T> event{ entity, { s, entity._index, entity._version } };
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

} // namespace secs

