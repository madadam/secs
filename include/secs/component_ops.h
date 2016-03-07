#pragma once

namespace secs {

class Entity;

class ComponentOps {
public:

  ComponentOps()
    : _copy(&transfer_noop)
    , _move(&transfer_noop)
    , _destroy(&destroy_noop)
  {}

  template<typename T>
  void setup() {
    _copy = &copy<T>;
    _move = &move<T>;
    _destroy = &destroy<T>;
  }

  explicit operator bool () const {
    return _destroy != &destroy_noop;
  }

  void copy(const Entity& source, const Entity& target) {
    assert(_copy);
    _copy(source, target);
  }

  void move(const Entity& source, const Entity& target) {
    assert(_move);
    _move(source, target);
  }

  void destroy(const Entity& entity) {
    assert(_destroy);
    _destroy(entity);
  }

private:

  template<typename T> static
  typename std::enable_if<std::is_copy_constructible<T>::value, void>::type
  copy(const Entity&, const Entity&);

  template<typename T> static
  typename std::enable_if<!std::is_copy_constructible<T>::value, void>::type
  copy(const Entity&, const Entity&);

  template<typename T> static void move(const Entity&, const Entity&);
  template<typename T> static void destroy(const Entity&);

  static void transfer_noop(const Entity&, const Entity&) {}
  static void destroy_noop(const Entity&) {}

private:

  using Transfer = void (*)(const Entity&, const Entity&);
  using Destroy  = void (*)(const Entity&);

  Transfer _copy    = nullptr;
  Transfer _move    = nullptr;
  Destroy  _destroy = nullptr;
};

} // namespace secs