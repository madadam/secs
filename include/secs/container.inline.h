#pragma once

#include "secs/component_ptr.h"
#include "secs/entity.h"
#include "secs/entity_view.h"

namespace secs {

// Container implementation

template<typename... Ts>
EntityView<Ts...> Container::all() {
  return { *this };
}

inline Entity Container::get(size_t index) {
  return Entity(*this, index, get_version(index));
}

template<typename T, typename... Ts>
Entity Container::create() {
  auto entity = create();
  create_components<T, Ts...>(entity._index);
  return entity;
}

template<typename T, typename... Ts>
Entity Container::create(T&& c, Ts&&... cs) {
  auto entity = create();
  create_components(entity._index, std::forward<T>(c), std::forward<Ts>(cs)...);
  return entity;
}

template<typename T, typename... Args>
void Container::create_component(size_t index, Args&&... args) {
  auto& store = get_store<T>();
  assert(!store.contains(index));

  _ops.get<T>().template setup<T>();
  store.emplace(index, std::forward<Args>(args)...);
  _event_manager.emit(OnCreate<T>{ get(index).component<T>() });
}

template<typename T0, typename T1, typename... Ts>
void Container::create_components(size_t index) {
  create_component<T0>(index);
  create_components<T1, Ts...>(index);
}

template<typename T>
void Container::create_components(size_t index) {
  create_component<T>(index);
}

template<typename T, typename... Ts>
void Container::create_components(size_t index, T&& c, Ts&&... cs) {
  create_component<typename std::decay<T>::type>(index, std::forward<T>(c));
  create_components(index, std::forward<Ts>(cs)...);
}

template<typename T>
void Container::destroy_component(size_t index) {
  auto& store = get_store<T>();
  assert(store.contains(index));

  _event_manager.emit(OnDestroy<T>{ get(index).component<T>() });
  store.erase(index);
}


// ComponentOps implementation
template<typename T>
typename std::enable_if<std::is_copy_constructible<T>::value, void>::type
ComponentOps::copy(const Entity& source, const Entity& target) {
  target.component<T>().create(*source.component<T>());
}

template<typename T>
typename std::enable_if<!std::is_copy_constructible<T>::value, void>::type
ComponentOps::copy(const Entity&, const Entity&) {
  assert(false);
}

template<typename T>
void ComponentOps::move(const Entity& source, const Entity& target) {
  target.component<T>().create(std::move(*source.component<T>()));
  source.component<T>().destroy();
}

template<typename T>
void ComponentOps::destroy(const Entity& entity) {
  entity.component<T>().destroy();
}


// ComponentPtr implementation
template<typename T>
Entity ComponentPtr<T>::entity() const {
  assert(valid());
  return _container->get(_index);
}


// Entity implementation
template<typename T>
ComponentPtr<T> Entity::component() const {
  assert(valid());
  return { *_container, _index, _version };
}

} // namespace secs

