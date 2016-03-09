#pragma once

#include <cassert>
#include <type_traits>

// Partially unsafe, type-erased storage for any type.
//
// Any can hold value of any type. It is safe to copy, move and destroy.
// Retrieval of the value is not safe, as there is no check that the retrieved
// type is the same as the stored type. This is reponsibility of the user.

namespace secs {

class Any {
private:
  template<typename T, typename R = void>
  using disable_if_self = typename std::enable_if<
    !std::is_same<typename std::decay<T>::type, Any>::value, R>::type;

  template<typename T>
  static void destroy(void* store) {
    delete reinterpret_cast<T*>(store);
  }

public:
  Any() {}

  template<typename T>
  Any( T&& value, disable_if_self<T, void*> = nullptr)
    : _store(new T(std::forward<T>(value)))
    , _destroy(&destroy<T>)
  {}

  ~Any() {
    reset();
  }

  Any(const Any&) = delete;
  Any(Any&&) noexcept;

  Any& operator = (const Any&) = delete;
  Any& operator = (Any&&);

  template<typename T>
  disable_if_self<T, Any&> operator = (T&& value) {
    emplace<T>(std::forward<T>(value));
    return *this;
  }

  template<typename T, typename... Args>
  void emplace(Args&&... args) {
    reset();
    _store = new T(std::forward<Args>(args)...);
    _destroy = &destroy<T>;
  }

  template<typename T>
  const T& get() const {
    assert(*this);
    return *reinterpret_cast<const T*>(_store);
  }

  template<typename T>
  T& get() {
    assert(*this);
    return *reinterpret_cast<T*>(_store);
  }

  explicit operator bool () const {
    return _store != nullptr;
  }

  // Test that this Any contains a value of type T.
  template<typename T>
  bool contains() const {
    return _destroy == &destroy<T>;
  }

  void reset();

private:
  using Destroy = void (*)(void*);

  void*   _store   = nullptr;
  Destroy _destroy = nullptr;
};

}; // namespace secs