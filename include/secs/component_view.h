#pragma once

#include <cassert>
#include "secs/component_store.h"

namespace secs {
namespace detail {

template<typename T>
class ComponentView {
  using StoreRef = typename std::conditional< std::is_const<T>::value
                                            , const ComponentStore&
                                            , ComponentStore&>::type;

public:
  bool contains(size_t index) const {
    return _store.contains(index);
  }

  const T& operator [] (size_t index) const {
    assert(contains(index));
    return _store.template get<T>(index);
  }

  size_t size() const {
    return _store.size();
  }

protected:
  ComponentView(StoreRef store)
    : _store(store)
  {}

protected:
  StoreRef _store;
};

template<typename T>
typename std::enable_if<std::is_default_constructible<T>::value>::type
handle_missing(ComponentStore& store, size_t index) {
  if (!store.contains(index)) {
      store.emplace<T>(index);
  }
}

template<typename T>
typename std::enable_if<!std::is_default_constructible<T>::value>::type
handle_missing(ComponentStore& store, size_t index) {
  assert(store.contains(index));
}

} // namespace detail

template<typename T>
class ConstComponentView : public detail::ComponentView<T> {
  using detail::ComponentView<T>::ComponentView;
  using detail::ComponentView<T>::_store;

public:

private:
  friend class EntityStore;
};

template<typename T>
class MutableComponentView: public detail::ComponentView<T> {
  using detail::ComponentView<T>::ComponentView;
  using detail::ComponentView<T>::_store;

public:

  using detail::ComponentView<T>::contains;
  using detail::ComponentView<T>::operator [];

  T& operator [] (size_t index) {
    detail::handle_missing<T>(_store, index);
    return _store.template get<T>(index);
  }

  template<typename... Args>
  void emplace(size_t index, Args&&... args) {
    _store.emplace<T>(index, std::forward<Args>(args)...);
  }

private:
  friend class EntityStore;
};

} // namespace secs