#pragma once

// Sequence of all Entities in a Container.

#include "secs/container.h"

namespace secs {

class EntityView {
public:
  class Iterator : public std::iterator<std::forward_iterator_tag, Entity> {
  public:
    bool operator == (const Iterator& other) const {
      return _index == other._index;
    }

    bool operator != (const Iterator& other) const {
      return _index != other._index;
    }

    Iterator& operator ++ () {
      advance(1);
      return *this;
    }

    Iterator& operator += (size_t distance) {
      advance(distance);
      return *this;
    }

    Entity operator * () const {
      return _container.get(_index);
    }

  private:
    Iterator(Container& container, size_t index)
      : _container(container)
      , _index(index)
    {
      advance(0);
    }

    void advance(size_t offset) {
      _index += offset;

      for (; _index < _container.capacity(); ++_index) {
        if (_container.contains(_index)) return;
      }

      _index = _container.capacity();
    }

  private:
    Container& _container;
    size_t     _index;

    friend class EntityView;
  };

  EntityView(Container& container)
    : _container(container)
  {}

  Iterator begin() const {
    return { _container, 0 };
  }

  Iterator end() const {
    return { _container, _container.capacity() };
  }

  bool   empty() const { return begin() == end(); }
  size_t size()  const { return _container.size(); }

  auto front() const { return *begin(); }

private:
  Container& _container;
  friend Container* get_container(const EntityView&);
};

inline Container* get_container(const EntityView& range) {
  return &range._container;
}

}
