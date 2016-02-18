#pragma once

#include <cstddef>
#include <cstdint>

namespace secs {

class EntityStore;

namespace detail {

class Handle {
public:
  Handle(std::nullptr_t)
    : _store(nullptr)
    , _index(0)
    , _version(0)
  {}

protected:

  Handle(EntityStore& store, size_t index, uint64_t version)
    : _store(&store)
    , _index(index)
    , _version(version)
  {}

  bool valid() const;

protected:
  EntityStore* _store;
  size_t       _index;
  uint64_t     _version;
};
} // namespace detail

template<typename Base>
class Handle : public detail::Handle {
public:
  using detail::Handle::Handle;

  Base& operator = (std::nullptr_t) {
    _store = nullptr;
    _index = 0;
    _version = 0;

    return static_cast<Base>(*this);
  }

  bool operator == (const Base& other) const {
    return _store == other._store
        && _index == other._index
        && _version == other._version;
  }

  bool operator != (const Base& other) const {
    return !(*this == other);
  }

  bool operator == (std::nullptr_t) const {
    return (bool) static_cast<Base>(*this);
  }

  bool operator != (std::nullptr_t) const {
    return !(*this == nullptr);
  }
};

} // namespace secs
