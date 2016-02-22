#pragma once

#include <memory>
#include <boost/dynamic_bitset.hpp>

namespace secs {

// Type-erased continuous storage for any type.
class ComponentStore {
public:

  ComponentStore() {}
  ComponentStore(const ComponentStore&) = delete;
  ComponentStore(ComponentStore&&) = default;

  ~ComponentStore() {
    for (size_t i = 0; i < size(); ++i) {
      if (_flags[i]) _deleter(_data, i);
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
    return contains(index) ? ptr<T>(_data, index) : nullptr;
  }

  template<typename T, typename... Args>
  T& emplace(size_t index, Args&&... args) {
    ensure_space<T>(index);

    if (_flags[index]) {
      ptr<T>(_data, index)->~T();
    }

    new (ptr<T>(_data, index)) T(std::forward<Args>(args)...);

    _flags[index] = true;

    if (!_deleter) {
      _deleter = [](std::unique_ptr<char[]>& data, size_t index) {
        ptr<T>(data, index)->~T();
      };
    }

    return *ptr<T>(_data, index);
  }

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
  static T* ptr(std::unique_ptr<char[]>& data, size_t index) {
    return reinterpret_cast<T*>(reinterpret_cast<Slot<T>*>(data.get()) + index);
  }

  // TODO: specialize this for PODs using memcpy
  template<typename T>
  static void move( std::unique_ptr<char[]>&       dst
                  , std::unique_ptr<char[]>&       src
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
    move<T>(new_data, _data, old_size, _flags);

    _data = std::move(new_data);
  }

private:

  boost::dynamic_bitset<> _flags;
  std::unique_ptr<char[]> _data;
  std::function<void(std::unique_ptr<char[]>&, size_t)> _deleter;
};

} // namespace secs