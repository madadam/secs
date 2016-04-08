#pragma once

#include <cmath>
#include <cstring>
#include <memory>
#include <type_traits>
#include <vector>

#include "secs/version.h"

namespace secs {
namespace detail {
  template<typename T>
  using Store = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

  template<typename T>
  T* ptr(Store<T>* data, size_t index) {
    return reinterpret_cast<T*>(data + index);
  }

  template<typename T>
  const T* ptr(const Store<T>* data, size_t index) {
    return reinterpret_cast<T*>(data + index);
  }

  template<typename T>
  std::enable_if_t<!std::is_trivially_copyable<T>::value, void>
  move( Store<T>*                   dst
      , Store<T>*                   src
      , size_t                      count
      , const std::vector<Version>& versions)
  {
    for (size_t i = 0; i < count; ++i) {
      if (versions[i].exists()) {
        new (ptr<T>(dst, i)) T(std::move(*ptr<T>(src, i)));
      }
    }
  }

  // If T is trivially copyable, we can move the memory much faster using
  // memcpy.
  template<typename T>
  std::enable_if_t<std::is_trivially_copyable<T>::value, void>
  move(Store<T>* dst, Store<T>* src, size_t count, const std::vector<Version>&) {
    std::memcpy( reinterpret_cast<void*>(dst)
               , reinterpret_cast<void*>(src)
               , count * sizeof(Store<T>));
  }

  template<typename T, typename... Args>
  std::enable_if_t<std::is_move_assignable<T>::value>
  replace(T& dst, Args&&... args) {
    dst = T(std::forward<Args>(args)...);
  }

  template<typename T, typename... Args>
  std::enable_if_t<!std::is_move_assignable<T>::value &&
                    std::is_copy_assignable<T>::value>
  replace(T& dst, Args&&... args) {
    T temp(std::forward<Args>(args)...);
    dst = temp;
  }

  template<typename T, typename... Args>
  std::enable_if_t<!std::is_move_assignable<T>::value &&
                   !std::is_copy_assignable<T>::value>
  replace(T& dst, Args&&... args) {
    dst.~T();
    new (&dst) T(std::forward<Args>(args)...);
  }

} // namespace detail

template<typename T>
class ComponentStore {
public:
  ComponentStore() = default;
  ComponentStore(const ComponentStore&) = delete;
  ComponentStore(ComponentStore&& other) = default;

  ~ComponentStore() {
    for (size_t i = 0; i < size(); ++i) {
      if (_versions[i].exists()) ptr(i)->~T();
    }
  }

  ComponentStore& operator = (const ComponentStore&) = delete;
  ComponentStore& operator = (ComponentStore&&) = default;

  size_t size() const {
    return _versions.size();
  }

  bool contains(size_t index, Version version) const {
    return index < _versions.size() && _versions[index] == version;
  }

  bool contains(size_t index) const {
    return index < _versions.size() && _versions[index].exists();
  }

  T& get(size_t index) {
    assert(contains(index));
    return *ptr(index);
  }

  const T& get(size_t index) const {
    assert(contains(index));
    return *ptr(index);
  }

  template<typename... Args>
  void emplace(size_t index, Version version, Args&&... args);

  void emplace(size_t index, Version version, const T& other);
  void emplace(size_t index, Version version, T& other);
  void emplace(size_t index, Version version, T&& other);

  void erase(size_t index) {
    if (!_versions[index].exists()) return;

    ptr(index)->~T();
    _versions[index].destroy();
  }


private:
  using Slot = detail::Store<T>;

  static constexpr double GROW_RATE = 1.5;

  T* ptr(size_t index) {
    return detail::ptr<T>(_data.get(), index);
  }

  const T* ptr(size_t index) const {
    return detail::ptr<T>(_data.get(), index);
  }

  void reserve_for(size_t index) {
    if (index < size()) return;

    auto old_size = size();
    auto new_size = std::max((size_t) std::ceil(GROW_RATE * old_size), index + 1);

    _versions.resize(new_size);

    auto new_data = std::make_unique<Slot[]>(new_size);
    detail::move<T>(new_data.get(), _data.get(), old_size, _versions);

    _data = std::move(new_data);
  }

  template<typename... Args>
  void emplace_without_invalidation_check( size_t    index
                                         , Version   version
                                         , Args&&... args)
  {
    reserve_for(index);

    if (_versions[index].exists()) {
      detail::replace(*ptr(index), std::forward<Args>(args)...);
    } else {
      new (ptr(index)) T(std::forward<Args>(args)...);
    }

    _versions[index] = version;
  }

  // Is inserting item into the given index going to invalidate the source
  // pointer?
  bool will_invalidate(size_t index, const T* source) const {
    return index >= size() && source >= ptr(0) && source < ptr(size());
  }

private:

  std::vector<Version>    _versions;
  std::unique_ptr<Slot[]> _data;
};

template<typename T> template<typename... Args>
void ComponentStore<T>::emplace( size_t    index
                               , Version   version
                               , Args&&... args)
{
  emplace_without_invalidation_check( index
                                    , version
                                    , std::forward<Args>(args)...);
}

// These overloads are necessary to prevent invalidating the source reference
// when the target storage has to be reallocated.

template<typename T>
void ComponentStore<T>::emplace(size_t index, Version version, T& other) {
  if (will_invalidate(index, &other)) {
    T temp(other);
    emplace_without_invalidation_check(index, version, std::move(temp));
  } else {
    emplace_without_invalidation_check(index, version, other);
  }
}

template<typename T>
void ComponentStore<T>::emplace(size_t index, Version version, const T& other) {
  if (will_invalidate(index, &other)) {
    T temp(other);
    emplace_without_invalidation_check(index, version, std::move(temp));
  } else {
    emplace_without_invalidation_check(index, version, other);
  }
}

template<typename T>
void ComponentStore<T>::emplace(size_t index, Version version, T&& other) {
  if (will_invalidate(index, &other)) {
    T temp(std::move(other));
    emplace_without_invalidation_check(index, version, std::move(temp));
  } else {
    emplace_without_invalidation_check(index, version, std::move(other));
  }
}

} // namespace secs
