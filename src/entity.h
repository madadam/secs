#pragma once

#include <cstddef>

namespace secs {

class EntityStore;

class Entity {
public:
  Entity(const Entity&) = delete;
  Entity(Entity&&) = default;

  Entity& operator = (const Entity&) = delete;
  Entity& operator = (Entity&&) = default;

  bool operator == (const Entity& other) const {
    return _store == other._store
        && _index == other._index
        && _version == other._version;
  }

  bool operator != (const Entity& other) const {
    return !(*this == other);
  }

  size_t id() const {
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

  template<typename T>
  void remove_component() const;

  void destroy() const;

private:

  Entity(EntityStore& store, size_t index, uint64_t version)
    : _store(&store)
    , _index(index)
    , _version(version)
  {}

  void check_version() const;

private:

  EntityStore* _store;
  size_t       _index;
  uint64_t     _version;

  friend class EntityStore;
};

} // namespace secs

#include "entity.inline.h"