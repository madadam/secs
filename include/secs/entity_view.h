#pragma once

#include "secs/misc.h"

namespace secs {

template<typename... Ts>
class EntityView {
public:
  class Iterator;

  class Cursor {
  public:
    const Entity& entity() const {
      return _entity;
    }

    template<typename T>
    T& get() const {
      return std::get<T&>(_components);
    }

  private:
    Cursor(Entity entity, Ts&... components)
      : _entity(entity)
      , _components(components...)
    {}

  private:
    const Entity             _entity;
    const std::tuple<Ts&...> _components;

    friend class Iterator;
  };

  class Iterator {
  public:
    bool operator == (Iterator other) const {
      return &_view == &other._view && _index == other._index;
    }

    bool operator != (Iterator other) const {
      return &_view != &other._view || _index != other._index;
    }

    Iterator& operator ++ () {
      advance(1);
      return *this;
    }

    Cursor operator * () const {
      return { _view._container.get(_index)
             , std::get<ComponentStore<Ts>&>(_view._stores).get(_index)... };
    }

  private:

    Iterator(const EntityView& view, size_t index)
      : _view(view)
      , _index(index)
    {
      advance(0);
    }

    void advance(size_t offset) {
      _index += offset;

      while (true) {
        if (_index >= _view._container.capacity()) {
          _index = _view._container.capacity();
          return;
        }

        if (_view.valid(_index)) {
          return;
        }

        ++_index;
      }
    }

  private:
    const EntityView<Ts...>& _view;
    size_t                   _index;

    friend class EntityView;
  };

public:
  EntityView(Container& container)
    : _container(container)
    , _stores(container._stores.slice<ComponentStore<Ts>...>())
  {}

  Iterator begin() const {
    return { *this, 0 };
  }

  Iterator end() const {
    return { *this, _container.capacity() };
  }

private:
  bool valid(size_t index) const {
    return all(_stores, [=](auto& store) { return store.contains(index); });
  }

private:
  Container&                         _container;
  std::tuple<ComponentStore<Ts>&...> _stores;
};

} // namespace secs