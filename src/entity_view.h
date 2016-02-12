#pragma once

#include <iterator>
#include "entity.h"

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
      return _env == other._env && _index == other._index;
    }

    bool operator != (iterator other) const {
      return _env != other._env || _index != other._index;
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
      return Entity(*_env, _index);
    }

  private:

    iterator(Environment& env, size_t index)
      : _env(&env)
      , _index(index)
    {
      advance(0);
    }

    void advance(size_t offset) {
      _index += offset;

      while (true) {
        if (_index >= _env->entity_capacity()) {
          _index = _env->entity_capacity();
          return;
        }

        if (_env->has_all_components<Ts...>(_index)) {
          return;
        }

        ++_index;
      }
    }

  private:

    Environment* _env;
    size_t       _index;

    friend class EntityView;
  };

  using value_type      = typename iterator::value_type;
  using reference       = typename iterator::reference;
  using difference_type = typename iterator::difference_type;
  using size_type       = size_t;

  EntityView(Environment& env)
    : _env(env)
  {}

  EntityView<Ts...>& operator = (const EntityView&) = delete;
  EntityView<Ts...>& operator = (EntityView&&) = delete;

  iterator begin() const {
    return iterator(_env, 0);
  }

  iterator end() const {
    return iterator(_env, _env.entities_capacity());
  }

private:

  Environment& _env;
};
