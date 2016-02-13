#pragma once

#include <stdexcept>

namespace secs {
namespace error {

class Base : public std::runtime_error {
public:
  Base(const std::string& what) : std::runtime_error(what) {}
};

class EntityInvalid : public Base {
public:
  EntityInvalid() : Base("Entity is invalid") {}
};

class ComponentNotFound : public Base {
public:
  ComponentNotFound() : Base("Component not found") {}
};

class ComponentAlreadyExists : public Base {
public:
  ComponentAlreadyExists() : Base("Component already exists") {}
};

} // namespace error
} // namespace secs