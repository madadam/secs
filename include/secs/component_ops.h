#pragma once

namespace secs {

class Entity;

class ComponentOps {
public:
  ComponentOps()
    : _copy(&copy_noop)
    , _destroy(&destroy_noop)
  {}

  template<typename T>
  void setup() {
    _copy    = &copy<T>;
    _destroy = &destroy<T>;
  }

  explicit operator bool () const {
    return _destroy != &destroy_noop;
  }

  void copy(const Entity& source, const Entity& target) {
    assert(_copy);
    _copy(source, target);
  }

  void destroy(const Entity& entity) {
    assert(_destroy);
    _destroy(entity);
  }

private:

  template<typename T> static
  std::enable_if_t<std::is_copy_constructible<T>::value, void>
  copy(const Entity&, const Entity&);

  template<typename T> static
  std::enable_if_t<!std::is_copy_constructible<T>::value, void>
  copy(const Entity&, const Entity&);

  template<typename T> static void destroy(const Entity&);

  static void copy_noop(const Entity&, const Entity&) {}
  static void destroy_noop(const Entity&) {}

private:

  using Copy     = void (*)(const Entity&, const Entity&);
  using Destroy  = void (*)(const Entity&);

  Copy    _copy    = nullptr;
  Destroy _destroy = nullptr;
};

} // namespace secs