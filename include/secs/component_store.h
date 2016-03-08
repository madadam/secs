#pragma once

#include <cstring>
#include <memory>
#include <boost/dynamic_bitset.hpp>

namespace secs {

namespace detail {
  template<typename T>
  using Slot = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

  template<typename T>
  T* ptr(Slot<T>* data, size_t index) {
    return reinterpret_cast<T*>(data + index);
  }

  template<typename T>
  const T* ptr(const Slot<T>* data, size_t index) {
    return reinterpret_cast<T*>(data + index);
  }

  template<typename T>
  typename std::enable_if<!std::is_trivially_copyable<T>::value, void>::type
  move( Slot<T>*                       dst
      , Slot<T>*                       src
      , size_t                         count
      , const boost::dynamic_bitset<>& flags)
  {
    for (size_t i = 0; i < count; ++i) {
      if (flags[i]) {
        new (ptr<T>(dst, i)) T(std::move(*ptr<T>(src, i)));
      }
    }
  }

  // If T is trivially copyable, we can move the memory much faster using
  // memcpy.
  template<typename T>
  typename std::enable_if<std::is_trivially_copyable<T>::value, void>::type
  move( Slot<T>*                       dst
      , Slot<T>*                       src
      , size_t                         count
      , const boost::dynamic_bitset<>&)
  {
    std::memcpy( reinterpret_cast<void*>(dst)
               , reinterpret_cast<void*>(src)
               , count * sizeof(Slot<T>));
  }

} // namespace detail

// TODO: If T is copyable, ComponentStore<T> should also be copyable.

template<typename T>
class ComponentStore {
public:
  ComponentStore() = default;
  ComponentStore(const ComponentStore&) = delete;
  ComponentStore(ComponentStore&& other) = default;

  ~ComponentStore() {
    for (size_t i = 0; i < size(); ++i) {
      if (_flags[i]) ptr(i)->~T();
    }
  }

  ComponentStore& operator = (const ComponentStore&) = delete;
  ComponentStore& operator = (ComponentStore&&) = default;

  size_t size() const {
    return _flags.size();
  }

  bool contains(size_t index) const {
    return index < _flags.size() && _flags[index];
  }

  T* get(size_t index) {
    return contains(index) ? ptr(index) : nullptr;
  }

  const T* get(size_t index) const {
    return contains(index) ? ptr(index) : nullptr;
  }

  template<typename... Args>
  void emplace(size_t index, Args&&... args);

  void emplace(size_t index, const T& other);
  void emplace(size_t index, T& other);
  void emplace(size_t index, T&& other);

  void erase(size_t index) {
    if (!contains(index)) return;

    ptr(index)->~T();
    _flags[index] = false;
  }


private:
  using Slot = detail::Slot<T>;

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

    _flags.resize(new_size);

    auto new_data = std::make_unique<Slot[]>(new_size);
    detail::move<T>(new_data.get(), _data.get(), old_size, _flags);

    _data = std::move(new_data);
  }

  template<typename... Args>
  void emplace_without_invalidation_check(size_t index, Args&&... args) {
    reserve_for(index);

    if (_flags[index]) {
      ptr(index)->~T();
    }

    new (ptr(index)) T(std::forward<Args>(args)...);

    _flags[index] = true;
  }

  // Is inserting item into the given index going to invalidate the source
  // pointer?
  bool will_invalidate(size_t index, const T* source) const {
    return index >= size() && source >= ptr(0) && source < ptr(size());
  }

private:

  boost::dynamic_bitset<> _flags;
  std::unique_ptr<Slot[]> _data;
};

template<typename T> template<typename... Args>
void ComponentStore<T>::emplace(size_t index, Args&&... args) {
  emplace_without_invalidation_check(index, std::forward<Args>(args)...);
}

// These overloads are necessary to prevent invalidating the source reference
// when the target storage has to be reallocated.

template<typename T>
void ComponentStore<T>::emplace(size_t index, T& other) {
  if (will_invalidate(index, &other)) {
    T temp(other);
    emplace_without_invalidation_check(index, std::move(temp));
  } else {
    emplace_without_invalidation_check(index, other);
  }
}

template<typename T>
void ComponentStore<T>::emplace(size_t index, const T& other) {
  if (will_invalidate(index, &other)) {
    T temp(other);
    emplace_without_invalidation_check(index, std::move(temp));
  } else {
    emplace_without_invalidation_check(index, other);
  }
}

template<typename T>
void ComponentStore<T>::emplace(size_t index, T&& other) {
  if (will_invalidate(index, &other)) {
    T temp(std::move(other));
    emplace_without_invalidation_check(index, std::move(temp));
  } else {
    emplace_without_invalidation_check(index, std::move(other));
  }
}

} // namespace secs
