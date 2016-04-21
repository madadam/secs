#pragma once

#include "secs/entity.h"

namespace secs {

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
std::enable_if_t<std::is_constructible<T, Args...>::value, ComponentPtr<T>>
Entity::create_component(Args&&... args) const {
  assert(*this);
  return _container->create_component<T>(*this, std::forward<Args>(args)...);
}

template<typename T>
void Entity::destroy_component() const {
  assert(*this);
  _container->destroy_component<T>(*this);
}

} // namespace secs