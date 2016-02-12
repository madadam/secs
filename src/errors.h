#pragma once

#include <stdexcept>

namespace error {

class Base : public std::runtime_error {
public:
  Base(const std::string& what) : std::runtime_error(what) {}
};

class ComponentNotFound : public Base {
public:
  ComponentNotFound() : Base("Not found") {}
};

class ComponentAlreadyExists : public Base {
public:
  ComponentAlreadyExists() : Base("Component already exists") {}
};

} // namespace error