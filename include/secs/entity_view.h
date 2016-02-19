#pragma once

#include <iterator>
#include "secs/entity.h"

namespace secs {

template<typename... Ts>
class EntityView {
public:

  struct iterator : public std::iterator< std::forward_iterator_tag
                                        , Entity
                                        , std::ptrdiff_t
                                        , Entity*
                                        , Entity>
  {
    bool operator == (iterator other) const {
      return _store == other._store && _index == other._index;
    }

    bool operator != (iterator other) const {
      return _store != other._store || _index != other._index;
    }

    iterator& operator ++ () {
      advance(1);
      return *this;
    }

    iterator operator ++ (int) {
      auto temp = *this;
      ++*this;
      return temp;
    }

    Entity operator * () const {
      return _store->get(_index);
    }

  private:

    iterator(EntityStore& store, size_t index)
      : _store(&store)
      , _index(index)
    {
      advance(0);
    }

    void advance(size_t offset) {
      _index += offset;

      while (true) {
        if (_index >= _store->capacity()) {
          _index = _store->capacity();
          return;
        }

        if (_store->has_all_components<Ts...>(_index)) {
          return;
        }

        ++_index;
      }
    }

  private:

    EntityStore* _store;
    size_t       _index;

    friend class EntityView;
  };

  using value_type      = typename iterator::value_type;
  using reference       = typename iterator::reference;
  using difference_type = typename iterator::difference_type;
  using size_type       = size_t;

  EntityView(EntityStore& store)
    : _store(store)
  {}

  EntityView(const EntityView&) = default;
  EntityView(EntityView&&) = default;

  EntityView<Ts...>& operator = (const EntityView&) = delete;
  EntityView<Ts...>& operator = (EntityView&&) = delete;

  iterator begin() const {
    return iterator(_store, 0);
  }

  iterator end() const {
    return iterator(_store, _store.capacity());
  }

  template<typename F>
  void each(F&& f) {
    for (auto entity : *this) {
      f(entity, entity.template get_component<Ts>()...);
    }
  }

  bool empty() const {
    return begin() == end();
  }

  Entity front() const {
    return *begin();
  }

  Entity create() {
    return _store.create<Ts...>();
  }

private:

  EntityStore& _store;
};

} // namespace secs