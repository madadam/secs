#pragma once

#include "entity_store.h"

namespace secs {

template<typename T>
bool Entity::has_component() const {
  check_version();
  return _store->has_component<T>(_index);
}

template<typename... Ts>
bool Entity::has_all_components() const {
  check_version();
  return _store->has_all_components<Ts...>(_index);
}

template<typename T>
T& Entity::get_component() const {
  check_version();
  return _store->get_component<T>(_index);
}

template<typename T>
T& Entity::get_or_add_component() const {
  check_version();
  return _store->get_or_add_component<T>(_index);
}

template<typename T>
T& Entity::add_component(T&& c) const {
  check_version();
  return _store->add_component<typename std::decay<T>::type>(_index, std::forward<T>(c));
}

template<typename T, typename... Args>
T& Entity::emplace_component(Args&&... args) const {
  check_version();
  return _store->emplace_component<T>(_index, std::forward<Args>(args)...);
}

template<typename T>
void Entity::remove_component() const {
  check_version();
  _store->remove_component<T>();
}

inline void Entity::destroy() const {
  check_version();
  _store->destroy(*this);
}

inline void Entity::check_version() const {
#ifndef NDEBUG
  if (_store->get_version(_index) != _version) {
    throw error::EntityInvalid();
  }
#endif
}

} // namespace secs