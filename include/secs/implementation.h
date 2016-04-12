#pragma once

#include "secs/component_ptr.h"
#include "secs/container.h"
#include "secs/entity.h"
#include "secs/entity_filter.h"
#include "secs/entity_view.h"
#include "secs/events.h"

namespace secs {
namespace detail {

// Test that T has on_create() member.
template<typename T>
struct has_on_create {
private:
  template<typename U>
  static constexpr decltype(std::declval<U>().on_create(Entity()), true)
  test(int) { return true; }

  template<typename>
  static constexpr bool test(...) { return false; }
public:
  static const bool value = test<T>(0);
};

// Test that T has on_destroy() member.
template<typename T>
struct has_on_destroy {
private:
  template<typename U>
  static constexpr decltype(std::declval<U>().on_destroy(Entity()), true)
  test(int) { return true; }

  template<typename>
  static constexpr bool test(...) { return false; }
public:
  static const bool value = test<T>(0);
};

// Test that T has on_change() member.
template<typename T>
struct has_on_change {
private:
  template<typename U>
  static constexpr decltype(std::declval<U>().on_change(Entity()), true)
  test(int) { return true; }

  template<typename>
  static constexpr bool test(...) { return false; }
public:
  static const bool value = test<T>(0);
};

template<typename T>
std::enable_if_t<has_on_create<T>::value>
invoke_on_create(const Entity& entity, T& component) {
  component.on_create(entity);
}

template<typename T>
std::enable_if_t<!has_on_create<T>::value>
invoke_on_create(const Entity&, T&) {}

template<typename T>
std::enable_if_t<has_on_destroy<T>::value>
invoke_on_destroy(const Entity& entity, T& component) {
  component.on_destroy(entity);
}

template<typename T>
std::enable_if_t<!has_on_destroy<T>::value>
invoke_on_destroy(const Entity&, T&) {}

} // namespace detail

template<typename... Ts>
inline EntityFilter<EntityView, Ts...> Container::entities() {
  return { *this };
}

inline Entity Container::get(size_t index) {
  return Entity(*this, index, get_version(index));
}

template<typename T, typename... Args>
ComponentPtr<T> Container::create_component( const Entity& entity
                                           , Args&&...     args)
{
  _ops.get<T>().template setup<T>();

  auto& s = store<T>();
  s.emplace(entity._index, entity._version, std::forward<Args>(args)...);

  ComponentPtr<T> component(s, entity._index, entity._version);

  detail::invoke_on_create(entity, *component);

  OnCreate<T> event{ entity, component };
  emit(entity, event);
  emit(event);

  return component;
}

template<typename T>
void Container::destroy_component(const Entity& entity) {
  auto& s = store<T>();
  if (!s.contains(entity._index)) return;

  ComponentPtr<T> component(s, entity._index, entity._version);

  detail::invoke_on_destroy(entity, *component);

  OnDestroy<T> event{ entity, component };
  emit(entity, event);
  emit(event);

  s.erase(entity._index);
}

template<typename E, typename F>
std::enable_if_t<detail::IsCallable<F, E&>, Connection<const E&>>
Container::connect(const Entity& entity, F&& f) {
  assert(contains(entity._index));
  return _meta[entity._index].signals.get<Signal<const E&>>()
                                     .connect(std::forward<F>(f));
}

template<typename E>
void Container::emit(const Entity& entity, const E& event) const {
  assert(contains(entity._index));
  _meta[entity._index].signals.get<Signal<const E&>>()(event);
}

// ComponentOps implementation
template<typename T>
std::enable_if_t<std::is_copy_constructible<T>::value, void>
ComponentOps::copy(const Entity& source, const Entity& target) {
  if (auto sc = source.component<T>()) {
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
  entity.destroy_component<T>();
}

// Entity implementation
template<typename... Ts>
Entity::Entity(const FilteredEntity<Ts...>& other)
  : Entity(other._entity)
{}

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

template<typename E, typename F>
std::enable_if_t<detail::IsCallable<F, E&>, Connection<const E&>>
Entity::connect(F&& f) {
  return _container->connect<E>(*this, std::forward<F>(f));
}

template<typename E>
void Entity::emit(const E& event) const {
  _container->emit(*this, event);
}

} // namespace secs
