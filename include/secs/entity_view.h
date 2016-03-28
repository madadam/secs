#pragma once

#include <tuple>
#include "secs/container.h"
#include "secs/component_set.h"
#include "secs/detail.h"

namespace secs {
namespace detail {
template<typename... Ts>
using ComponentStores = std::tuple<ComponentStore<Ts>*...>;

template<typename S>
using Iterator = decltype(std::begin(std::declval<S>()));

template<typename Source, typename... Us>
using Load = decltype(std::declval<Source>().template load<Us...>());

}

template<typename, typename...> class NeedEntityView;
template<typename, typename...> class SkipEntityView;

////////////////////////////////////////////////////////////////////////////////
// View of all Entities in a Container.
template<typename... Ts>
class ContainerEntityView {
public:
  class Iterator;

  class Enumerator {
  public:
    const Entity& entity() const {
      return _entity;
    }

    template<typename T>
    decltype(auto) get() const {
      return _components.get<T>();
    }

    const ComponentSet<Ts...>& all() const {
      return _components;
    }

  private:
    Enumerator(const Entity& entity, const detail::ComponentStores<Ts...>& stores)
      : _entity(entity)
      , _components(entity.components(stores))
    {}

  private:
    Entity              _entity;
    ComponentSet<Ts...> _components;
    friend class Iterator;
  };

  class Iterator {
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

    Enumerator operator * () const {
      return { _container.get(_index), _stores };
    }

  private:
    Iterator( Container&                            container
            , const detail::ComponentStores<Ts...>& stores
            , size_t                                index)
      : _container(container)
      , _stores(stores)
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
    Container&                     _container;
    detail::ComponentStores<Ts...> _stores;
    size_t                         _index;

    friend class ContainerEntityView;
  };

  ContainerEntityView(Container& container)
    : _container(container)
  {}

  Iterator begin() const {
    return { _container
           , _container.store_ptrs<Ts...>()
           , 0 };
  }

  Iterator end() const {
    return { _container
           , _container.store_ptrs<Ts...>()
           , _container.capacity() };
  }

  bool empty() { return begin() == end(); }
  auto front() { return *begin(); }

  template<typename... Us>
  ContainerEntityView<Ts..., Us...> load() const {
    return { _container };
  }

  template<typename... Us>
  NeedEntityView<ContainerEntityView<Ts..., Us...>, Us...> need() const {
    return { load<Us...>() };
  }

  template<typename... Us>
  SkipEntityView<ContainerEntityView<Ts..., Us...>, Us...> skip() const {
    return { load<Us...>() };
  }

private:
  Container& _container;
};

////////////////////////////////////////////////////////////////////////////////
// View of all Entities in a sequence of Entities (e.g. std::vector<Entity>))
template<typename Source, typename... Ts>
class SequenceEntityView {
public:
  using value_type = decltype(*std::begin(std::declval<Source>()));

  static_assert(std::is_convertible<value_type, Entity>::value,
                "Source is not a sequence of Entities");

  class Iterator;

  class Enumerator {
  public:
    const Entity& entity() const {
      return _entity;
    }

    template<typename T>
    decltype(auto) get() const {
      return all().get<T>();
    }

    ComponentSet<Ts...> all() const {
      return _entity.components<Ts...>();
    }

  private:
    Enumerator(const Entity& entity)
      : _entity(entity)
    {}

  private:
    Entity _entity;
    friend class Iterator;
  };

  class Iterator {
  public:
    bool operator != (const Iterator& other) const {
      return _source != other._source;
    }

    Enumerator operator * () const {
      return { *_source };
    }

    Iterator& operator ++ () {
      ++_source;
      return *this;
    }

    Iterator& operator += (size_t offset) {
      _source += offset;
      return *this;
    }

  private:
    Iterator(detail::Iterator<Source> source)
      : _source(std::move(source))
    {}

  private:
    detail::Iterator<Source> _source;
    friend class SequenceEntityView;
  };

public:
  SequenceEntityView(Source source)
    : _source(source)
  {}

  Iterator begin() const {
    return { std::begin(_source) };
  }

  Iterator end() const {
    return { std::end(_source) };
  }

  bool empty() { return begin() == end(); }
  auto front() { return *begin(); }

  template<typename... Us>
  SequenceEntityView<Source, Ts..., Us...> load() const {
    return { _source };
  }

  template<typename... Us>
  NeedEntityView<SequenceEntityView<Source, Ts..., Us...>, Us...> need() const {
    return { load<Us...>() };
  }

  template<typename... Us>
  SkipEntityView<SequenceEntityView<Source, Ts..., Us...>, Us...> skip() const {
    return { load<Us...>() };
  }

private:
  Source _source;
};

inline SequenceEntityView<const std::vector<Entity>&>
make_entity_view(const std::vector<Entity>& entities) {
  return { entities };
}

////////////////////////////////////////////////////////////////////////////////
// Base class for Filtered view of Entities
template<typename Base, typename Source>
class FilteredEntityView {
public:
  class Iterator {
  public:
    bool operator != (const Iterator& other) const {
      return _source != other._source;
    }

    Iterator& operator ++ () {
      advance(1);
      return *this;
    }

    auto operator * () const {
      return *_source;
    }

  private:
    Iterator(detail::Iterator<Source> source, detail::Iterator<Source> end)
      : _source(std::move(source))
      , _end(std::move(end))
    {
      advance(0);
    }

    void advance(size_t offset) {
      _source += offset;

      for (; _source != _end; ++_source) {
        if (Base::filter(*_source)) return;
      }
    }

  private:
    detail::Iterator<Source> _source;
    detail::Iterator<Source> _end;

    friend class FilteredEntityView;
  };

  using value_type = decltype(*std::declval<Iterator>());

public:
  FilteredEntityView(Source source)
    : _source(source)
  {
    // TODO: static assert that Source loads Ts
  }

  Iterator begin() const {
    return { std::begin(_source), std::end(_source) };
  }

  Iterator end() const {
    auto e = std::end(_source);
    return { e, e };
  }

protected:
  Source _source;
};

////////////////////////////////////////////////////////////////////////////////
// View of Entities that contain the given Components
template<typename Source, typename... Ts>
class NeedEntityView
  : public FilteredEntityView<NeedEntityView<Source, Ts...>, Source>
{
public:
  using Base = FilteredEntityView<NeedEntityView<Source, Ts...>, Source>;
  using Base::FilteredEntityView;
  using value_type = typename Base::value_type;

  template<typename... Us>
  NeedEntityView<detail::Load<Source, Us...>, Ts...> load() const {
    return { _source.load<Us...>() };
  }

  template<typename... Us>
  NeedEntityView<detail::Load<Source, Us...>, Ts..., Us...> need() const {
    return { _source.load<Us...>() };
  }

  template<typename... Us>
  SkipEntityView<detail::Load<Source, Us...>, Us...> skip() const {
    return { _source.load<Us...>() };
  }

  static bool filter(const value_type& e) {
    return e.all().template contains_all<Ts...>();
  }

private:
  using FilteredEntityView<NeedEntityView<Source, Ts...>, Source>::_source;
};

////////////////////////////////////////////////////////////////////////////////
// View of Entities that do not contain the given Components
template<typename Source, typename... Ts>
class SkipEntityView
  : public FilteredEntityView<SkipEntityView<Source, Ts...>, Source>
{
public:
  using Base = FilteredEntityView<SkipEntityView<Source, Ts...>, Source>;
  using Base::FilteredEntityView;
  using value_type = typename Base::value_type;

  template<typename... Us>
  SkipEntityView<detail::Load<Source, Us...>, Ts...> load() const {
    return { _source.load<Us...>() };
  }

  template<typename... Us>
  NeedEntityView<detail::Load<Source, Us...>, Us...> need() const {
    return { _source.load<Us...>() };
  }

  template<typename... Us>
  SkipEntityView<detail::Load<Source, Us...>, Ts..., Us...> skip() const {
    return { _source.load<Us...>() };
  }

  static bool filter(const value_type& e) {
    return e.all().template contains_none<Ts...>();
  }

private:
  using FilteredEntityView<SkipEntityView<Source, Ts...>, Source>::_source;
};

} // namespace secs