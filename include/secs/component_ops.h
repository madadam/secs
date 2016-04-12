#pragma once

namespace secs {

class Entity;

class ComponentOps {
public:
  template<typename T>
  void setup() {
    _copy    = &copy<T>;
    _destroy = &destroy<T>;
  }

  explicit operator bool () const {
    return _destroy != &noop1;
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

  static void noop2(const Entity&, const Entity&) {}
  static void noop1(const Entity&) {}

private:

  using Fun2 = void (*)(const Entity&, const Entity&);
  using Fun1 = void (*)(const Entity&);

  Fun2 _copy    = &noop2;
  Fun1 _destroy = &noop1;
};

} // namespace secs