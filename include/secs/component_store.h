#pragma once

#include <memory>
#include <boost/dynamic_bitset.hpp>

namespace secs {

class Container;

// Type-erased continuous storage for any type.
class ComponentStore {
private:

  // Type-erased operations on the objects stored in this ComponentStore.
  struct BaseOps {
    virtual void copy( const char* source_data
                     , size_t      source_index
                     , Container&  target
                     , size_t      target_index) = 0;
    virtual void move( const char* source_data
                     , size_t      source_index
                     , Container&  target
                     , size_t      target_index) = 0;
    virtual void erase(const char* data, size_t index) = 0;
  };

  template<typename T>
  struct Ops : BaseOps {
    void copy(const char*, size_t, Container&, size_t) override;
    void move(const char*, size_t, Container&, size_t) override;
    void erase(const char*, size_t) override;
  };

public:

  ComponentStore() {}
  ComponentStore(const ComponentStore&) = delete;
  ComponentStore(ComponentStore&&) = default;

  ~ComponentStore() {
    for (size_t i = 0; i < size(); ++i) {
      if (_flags[i]) _ops->erase(_data.get(), i);
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

  template<typename T>
  T* get(size_t index) {
    return contains(index) ? ptr<T>(index) : nullptr;
  }

  template<typename T, typename... Args>
  void emplace(size_t index, Args&&... args);

  template<typename T> void emplace(size_t index, const T& other);
  template<typename T> void emplace(size_t index, T& other);
  template<typename T> void emplace(size_t index, T&& other);

  template<typename T>
  void reserve_for(size_t index) {
    if (index < size()) return;

    auto old_size = size();
    auto new_size = std::max((size_t) std::ceil(GROW_RATE * old_size), index + 1);

    _flags.resize(new_size);

    auto new_data = std::make_unique<char[]>(slot_size<T>() * new_size);
    move<T>(new_data.get(), _data.get(), old_size, _flags);

    _data = std::move(new_data);
  }

  void copy(size_t source_index, Container& target_container, size_t target_index);
  void move(size_t source_index, Container& target_container, size_t target_index);
  void erase(size_t index);

private:

  static constexpr double GROW_RATE = 1.5;

  template<typename T>
  using Slot = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

  template<typename T>
  static constexpr size_t slot_size() {
    return sizeof(Slot<T>);
  }

  template<typename T>
  static const T* ptr(const char* data, size_t index) {
    return reinterpret_cast<const T*>(reinterpret_cast<const Slot<T>*>(data) + index);
  }

  template<typename T>
  static T* ptr(char* data, size_t index) {
    return reinterpret_cast<T*>(reinterpret_cast<Slot<T>*>(data) + index);
  }

  template<typename T>
  const T* ptr(size_t index) const {
    return ptr<T>(_data.get(), index);
  }

  template<typename T>
  T* ptr(size_t index) {
    return ptr<T>(_data.get(), index);
  }

  // TODO: specialize this for PODs using memcpy
  template<typename T>
  static void move( char*                          dst
                  , char*                          src
                  , size_t                         count
                  , const boost::dynamic_bitset<>& flags)
  {
    for (size_t i = 0; i < count; ++i) {
      if (flags[i]) {
        new (ptr<T>(dst, i)) T(std::move(*ptr<T>(src, i)));
      }
    }
  }

  // Is inserting item into the given index going to invalidate the source
  // pointer?
  template<typename T>
  bool will_invalidate(size_t index, const T* source) const {
    return index >= size() && source >= ptr<T>(0) && source < ptr<T>(size());
  }

  template<typename T, typename... Args>
  void emplace_without_invalidation_check(size_t index, Args&&... args) {
    reserve_for<T>(index);

    if (_flags[index]) {
      ptr<T>(index)->~T();
    }

    new (ptr<T>(index)) T(std::forward<Args>(args)...);

    _flags[index] = true;

    if (!_ops) {
      _ops = std::make_unique<Ops<T>>();
    }
  }

private:

  boost::dynamic_bitset<>  _flags;
  std::unique_ptr<char[]>  _data;
  std::unique_ptr<BaseOps> _ops = nullptr;
};

template<typename T, typename... Args>
void ComponentStore::emplace(size_t index, Args&&... args) {
  emplace_without_invalidation_check<T>(index, std::forward<Args>(args)...);
}

// These overloads are necessary to prevent invalidating the source reference
// when the target storage has to be reallocated.

template<typename T>
void ComponentStore::emplace(size_t index, T& other) {
  if (will_invalidate(index, &other)) {
    T temp(other);
    emplace_without_invalidation_check<T>(index, std::move(temp));
  } else {
    emplace_without_invalidation_check<T>(index, other);
  }
}

template<typename T>
void ComponentStore::emplace(size_t index, const T& other) {
  if (will_invalidate(index, &other)) {
    T temp(other);
    emplace_without_invalidation_check<T>(index, std::move(temp));
  } else {
    emplace_without_invalidation_check<T>(index, other);
  }
}

template<typename T>
void ComponentStore::emplace(size_t index, T&& other) {
  if (will_invalidate(index, &other)) {
    T temp(std::move(other));
    emplace_without_invalidation_check<T>(index, std::move(temp));
  } else {
    emplace_without_invalidation_check<T>(index, std::move(other));
  }
}

} // namespace secs
