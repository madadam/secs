#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

namespace secs {

class Container;

namespace detail {

class Handle {
public:
  Handle(std::nullptr_t)
    : _container(nullptr)
    , _index(0)
    , _version(0)
  {}

protected:

  Handle(Container& container, size_t index, uint64_t version)
    : _container(&container)
    , _index(index)
    , _version(version)
  {}

  bool valid() const;

protected:
  Container* _container;
  size_t     _index;
  uint64_t   _version;
};
} // namespace detail

template<typename Base>
class Handle : public detail::Handle {
public:
  using detail::Handle::Handle;

  Base& operator = (std::nullptr_t) {
    _container = nullptr;
    _index = 0;
    _version = 0;

    return static_cast<Base>(*this);
  }

  bool operator == (const Base& other) const {
    return _container == other._container
        && _index     == other._index
        && _version   == other._version;
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

  template<typename T>
  friend bool operator < (const Handle<T>&, const Handle<T>&);
};

// Define comparison operator so Handles can be used as keys in std::sets and
// std::maps.
template<typename Base>
bool operator < (const Handle<Base>& a, const Handle<Base>& b) {
  return std::make_pair(a._index, a._version)
       < std::make_pair(b._index, b._version);
}

} // namespace secs
