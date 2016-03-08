#pragma once

#include <cassert>
#include <type_traits>

// Partially unsafe, type-erased storage for any type.
//
// Any can hold value of any type. It is safe to copy, move and destroy.
// Retrieval of the value is not safe, as there is no check that the retrieved
// type is the same as the stored type. This is reponsibility of the user.

namespace secs {

class Any;

namespace detail {
template<typename T>
constexpr bool is_any = std::is_same<typename std::decay<T>::type, Any>::value;
}

class Any {
private:

  template<typename T>
  static
  typename std::enable_if<std::is_copy_constructible<T>::value, void*>::type
  copy(void* store) {
    return new T(*reinterpret_cast<T*>(store));
  }

  template<typename T>
  static
  typename std::enable_if<!std::is_copy_constructible<T>::value, void*>::type
  copy(void*) {
    assert(false);
    return nullptr;
  }

  template<typename T>
  static void destroy(void* store) {
    delete reinterpret_cast<T*>(store);
  }

public:
  Any() {}

  template<typename T>
  Any( T&& value
     , typename std::enable_if<!detail::is_any<T>, void*>::type = nullptr)
    : _store(new T(std::forward<T>(value)))
    , _destroy(&destroy<T>)
    , _copy(&copy<T>)
  {}

  ~Any() {
    reset();
  }

  Any(const Any&);
  Any(Any&&) noexcept;

  Any& operator = (const Any&);
  Any& operator = (Any&&);

  template<typename T>
  typename std::enable_if<!detail::is_any<T>, Any&>::type
  operator = (T&& value) {
    reset();

    _store = new T(std::forward<T>(value));
    _destroy = &destroy<T>;
    _copy = &copy<T>;

    return *this;
  }

  template<typename T>
  const T& get() const {
    assert(!empty());
    return *reinterpret_cast<const T*>(_store);
  }

  template<typename T>
  T& get() {
    if (empty()) {
      *this = T();
    }

    return *reinterpret_cast<T*>(_store);
  }

  bool empty() const {
    return _store == nullptr;
  }

  // Test that this Any contains a value of type T.
  template<typename T>
  bool contains() const {
    return _destroy == &destroy<T>;
  }

  void reset();

private:

  static void* copy(const Any&);

private:

  using Destroy = void  (*)(void*);
  using Copy    = void* (*)(void*);

  void*   _store   = nullptr;
  Destroy _destroy = nullptr;
  Copy    _copy    = nullptr;
};

}; // namespace secs