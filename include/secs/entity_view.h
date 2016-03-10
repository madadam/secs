#pragma once

#include "secs/container.h"
#include "secs/misc.h"

namespace secs {

namespace detail {

// Convert tuple<T...> to tuple<ComponentStore<T>&...>
namespace help {
template<typename... Ts>
struct ComponentStoreRefs;

template<typename... Ts>
struct ComponentStoreRefs<std::tuple<Ts...>> {
  using type = std::tuple<ComponentStore<Ts>&...>;
};
}

template<typename T>
using ComponentStoreRefs = typename help::ComponentStoreRefs<T>::type;

// Convert tuple<T...> to tuple<T&...>
namespace help {
template<typename... Ts>
struct ComponentRefs;

template<typename... Ts>
struct ComponentRefs<std::tuple<Ts...>> {
  using type = std::tuple<Ts&...>;
};
}

template<typename T>
using ComponentRefs = typename help::ComponentRefs<T>::type;

// Test if all stores have component at the given index.
template<typename T>
bool has_all(const ComponentStoreRefs<T>& stores, size_t index) {
  return secs::all(stores, [=](auto& store) {
    return store.contains(index);
  });
}

// Test if no store has component at the given index.
template<typename T>
bool has_none(const ComponentStoreRefs<T>& stores, size_t index) {
  return secs::all(stores, [=](auto& store) {
    return !store.contains(index);
  });
}

// Get tuple of index-th components, one from each store in the given store
// tuple.
namespace help {
template<typename T>
struct Get;

template<typename... Ts>
struct Get<std::tuple<Ts...>> {
  auto operator () ( const detail::ComponentStoreRefs<std::tuple<Ts...>>& stores
                   , size_t index) const
  {
    (void) index; // supress unused warning
    return std::tie(std::get<ComponentStore<Ts>&>(stores).get(index)...);
  }
};
}

template<typename T>
auto get(const ComponentStoreRefs<T>& stores, size_t index) {
  return help::Get<T>()(stores, index);
}

// Call set.slice, but use the types from the tuple T
namespace help {
template<typename T>
struct Slice;

template<typename... Ts>
struct Slice<std::tuple<Ts...>> {
  auto operator () (const Omniset& set) const {
    return set.slice<ComponentStore<Ts>...>();
  }
};
}

template<typename T>
auto slice(const Omniset& set) {
  return help::Slice<T>()(set);
}

// Base EntityView
template<typename Include, typename Exclude>
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
    Cursor(Entity entity, const ComponentRefs<Include>& components)
      : _entity(entity)
      , _components(components)
    {}

  private:
    const Entity                         _entity;
    const detail::ComponentRefs<Include> _components;

    friend class Iterator;
  };

  class Iterator {
  public:
    bool operator != (Iterator other) const {
      return _index != other._index;
    }

    Iterator& operator ++ () {
      advance(1);
      return *this;
    }

    Cursor operator * () const {
      return { _container.get(_index)
             , detail::get<Include>(_include, _index) };
    }

  private:
    Iterator( Container&                         container
            , const ComponentStoreRefs<Include>& include
            , const ComponentStoreRefs<Exclude>& exclude
            , size_t                             index)
      : _container(container)
      , _include(include)
      , _exclude(exclude)
      , _index(index)
    {
      advance(0);
    }

    void advance(size_t offset) {
      _index += offset;

      for (; _index < _container.capacity(); ++_index) {
        if (  _container.contains(_index)
           && has_all <Include>(_include, _index)
           && has_none<Exclude>(_exclude, _index))
        {
          return;
        }
      }

      _index = _container.capacity();
    }

  private:
    Container&                  _container;
    ComponentStoreRefs<Include> _include;
    ComponentStoreRefs<Exclude> _exclude;
    size_t                      _index;

    friend class EntityView;
  };

public:

  EntityView(Container& container)
    : _container(container)
    , _include(slice<Include>(container._stores))
    , _exclude(slice<Exclude>(container._stores))
  {}

  Iterator begin() {
    return { _container, _include, _exclude, 0 };
  }

  Iterator end() {
    return { _container, _include, _exclude, _container.capacity() };
  }

protected:
  Container&                  _container;
  ComponentStoreRefs<Include> _include;
  ComponentStoreRefs<Exclude> _exclude;
};

} // namespace detail

// Entities that have all the Include'd components and none of the Exclude'd
// components.
template<typename Include, typename Exclude>
class EntityView : public detail::EntityView<Include, Exclude> {
  using Base = detail::EntityView<Include, Exclude>;
  using Base::EntityView;
};

// All entities
template<>
class EntityView<std::tuple<>, std::tuple<>>
  : public detail::EntityView<std::tuple<>, std::tuple<>>
{
  using Base = detail::EntityView<std::tuple<>, std::tuple<>>;
  using Base::EntityView;
  using Base::_container;
public:

  template<typename... Ts>
  EntityView<std::tuple<Ts...>, std::tuple<>> with() const {
    return { _container };
  }

  template<typename... Ts>
  EntityView<std::tuple<>, std::tuple<Ts...>> without() const {
    return { _container };
  }
};

// Only entities with the given components
template<typename Include>
class EntityView<Include, std::tuple<>>
  : public detail::EntityView<Include, std::tuple<>>
{
  using Base = detail::EntityView<Include, std::tuple<>>;
  using Base::EntityView;
  using Base::_container;
public:

  template<typename... Ts>
  EntityView<Include, std::tuple<Ts...>> without() const {
    return { _container };
  }
};

} // namespace secs