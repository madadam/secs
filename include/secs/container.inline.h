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
  // TODO: emit on_component_create
  auto cs = components<T>();
  assert(!cs.contains(index));
  cs.emplace(index, std::forward<Args>(args)...);
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

// ComponentStore implementation

template<typename T>
void ComponentStore::Ops<T>::copy( const char* source_data
                                 , size_t      source_index
                                 , Container&  target
                                 , size_t      target_index)
{
  target.get(target_index)
        .component<T>()
        .create(*ptr<T>(source_data, source_index));
}

template<typename T>
void ComponentStore::Ops<T>::move( const char* source_data
                                 , size_t      source_index
                                 , Container&  target
                                 , size_t      target_index)
{
  target.get(target_index)
        .component<T>()
        .create(std::move(*ptr<T>(source_data, source_index)));
  ptr<T>(source_data, source_index)->~T();
}

template<typename T>
void ComponentStore::Ops<T>::erase(const char* data, size_t index) {
  ptr<T>(data, index)->~T();
}

// Entity implementation

template<typename T>
ComponentPtr<T> Entity::component() const {
  assert(valid());
  return { *_container, _index, _version };
}

} // namespace secs

