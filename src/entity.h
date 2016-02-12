#pragma once

class Environment;

class Entity {
public:

  Entity(Environment& env, size_t index)
    : _env(&env)
    , _index(index)
  {}

  size_t index() const {
    return _index;
  }

  template<typename T>
  bool has_component() const;

  template<typename... Ts>
  bool has_all_components() const;

  template<typename T>
  T& get_component() const;

  template<typename T>
  T& get_or_add_component() const;

  template<typename T>
  T& add_component(T&& c) const;

  template<typename T, typename... Args>
  T& emplace_component(Args&&... args) const;

private:

  Environment* _env;
  size_t       _index;
};

#include "entity.inline.h"