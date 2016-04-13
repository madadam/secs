#pragma once

#include "secs/component_ptr.h"
#include "secs/container.h"
#include "secs/entity.h"
#include "secs/entity_filter.h"
#include "secs/entity_view.h"
#include "secs/lifetime_events.h"

namespace secs {
namespace detail {

// Test that T has on_create() member.
template<typename T>
struct HasOnCreate {
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
struct HasOnDestroy {
private:
  template<typename U>
  static constexpr decltype(std::declval<U>().on_destroy(Entity()), true)
  test(int) { return true; }

  template<typename>
  static constexpr bool test(...) { return false; }
public:
  static const bool value = test<T>(0);
};

template<typename T>
std::enable_if_t<HasOnCreate<T>::value>
invoke_on_create(const Entity& entity, T& component) {
  component.on_create(entity);
}

template<typename T>
std::enable_if_t<!HasOnCreate<T>::value>
invoke_on_create(const Entity&, T&) {}

template<typename T>
std::enable_if_t<HasOnDestroy<T>::value>
invoke_on_destroy(const Entity& entity, T& component) {
  component.on_destroy(entity);
}

template<typename T>
std::enable_if_t<!HasOnDestroy<T>::value>
invoke_on_destroy(const Entity&, T&) {}

// template<typename T, typename E>

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
  emit(OnCreate<T>{ entity, component });

  return component;
}

template<typename T>
void Container::destroy_component(const Entity& entity) {
  auto& s = store<T>();
  if (!s.contains(entity._index)) return;

  ComponentPtr<T> component(s, entity._index, entity._version);
  detail::invoke_on_destroy(entity, *component);
  emit(OnDestroy<T>{ entity, component });

  s.erase(entity._index);
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

} // namespace secs

template<typename E, typename T>
std::enable_if_t<secs::CanHandleEvent<T, E>, secs::Connection>
secs::Container::connect() {
  using secs::event_entity;
  using secs::event_handle;

  return connect<E>([](auto& event) {
    if (auto component = event_entity(event).template component<T>()) {
      event_handle(*component, event);
    }
  });
}
