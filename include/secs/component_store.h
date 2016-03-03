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
  T& emplace(size_t index, Args&&... args) {
    ensure_space<T>(index);

    if (_flags[index]) {
      ptr<T>(index)->~T();
    }

    new (ptr<T>(index)) T(std::forward<Args>(args)...);

    _flags[index] = true;

    if (!_ops) {
      _ops = std::make_unique<Ops<T>>();
    }

    return *ptr<T>(index);
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
                  , const char*                    src
                  , size_t                         count
                  , const boost::dynamic_bitset<>& flags)
  {
    for (size_t i = 0; i < count; ++i) {
      if (flags[i]) {
        new (ptr<T>(dst, i)) T(std::move(*ptr<T>(src, i)));
      }
    }
  }

  template<typename T>
  void ensure_space(size_t index) {
    if (index < size()) return;

    auto old_size = size();
    auto new_size = std::max((size_t) std::ceil(GROW_RATE * old_size), index + 1);

    _flags.resize(new_size);

    auto new_data = std::make_unique<char[]>(slot_size<T>() * new_size);
    move<T>(new_data.get(), _data.get(), old_size, _flags);

    _data = std::move(new_data);
  }

private:

  boost::dynamic_bitset<>  _flags;
  std::unique_ptr<char[]>  _data;
  std::unique_ptr<BaseOps> _ops = nullptr;
};

} // namespace secs
