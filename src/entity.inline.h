#pragma once

#include "environment.h"

template<typename T>
bool Entity::has_component() const {
  return _env->has_component<T>(_index);
}

template<typename... Ts>
bool Entity::has_all_components() const {
  return _env->has_all_components<Ts...>(_index);
}

template<typename T>
T& Entity::get_component() const {
  return _env->get_component<T>(_index);
}

template<typename T>
T& Entity::get_or_add_component() const {
  return _env->get_or_add_component<T>(_index);
}

template<typename T>
T& Entity::add_component(T&& c) const {
  return _env->add_component<typename std::decay<T>::type>(_index, std::forward<T>(c));
}

template<typename T, typename... Args>
T& Entity::emplace_component(Args&&... args) const {
  return _env->emplace_component<T>(_index, std::forward<Args>(args)...);
}
