#pragma once

#include "secs/container.h"
#include "secs/detail.h"

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
struct ToRefs;

template<typename... Ts>
struct ToRefs<std::tuple<Ts...>> {
  using type = std::tuple<typename std::add_lvalue_reference<Ts>::type...>;
};
}

template<typename T>
using ToRefs = typename help::ToRefs<T>::type;


// Convert tuple<T...> to tuple<T*...>
namespace help {
template<typename... Ts>
struct ToPtrs;

template<typename... Ts>
struct ToPtrs<std::tuple<Ts...>> {
  using type = std::tuple<typename std::add_pointer<Ts>::type...>;
};
}

template<typename T>
using ToPtrs = typename help::ToPtrs<T>::type;


// Test if all stores have component at the given index.
template<typename T>
bool has_all(const ComponentStoreRefs<T>& stores, size_t index) {
  return all(stores, [=](auto& store) {
    return store.contains(index);
  });
}

// Test if no store has component at the given index.
template<typename T>
bool has_none(const ComponentStoreRefs<T>& stores, size_t index) {
  return all(stores, [=](auto& store) {
    return !store.contains(index);
  });
}

// Get tuple of references to index-th components, one from each store in the
// given store tuple.
namespace help {
template<typename T>
struct GetRefs;

template<typename... Ts>
struct GetRefs<std::tuple<Ts...>> {
  auto operator () ( const detail::ComponentStoreRefs<std::tuple<Ts...>>& stores
                   , size_t index) const
  {
    (void) index; // supress unused warning
    return std::tie(std::get<ComponentStore<Ts>&>(stores).get(index)...);
  }
};
}

template<typename T>
auto get_refs(const ComponentStoreRefs<T>& stores, size_t index) {
  return help::GetRefs<T>()(stores, index);
}


// Get pointer to component from the store, or nullptr it there is no component.
template<typename T>
T* get_ptr(ComponentStore<T>& store, size_t index) {
  return store.contains(index) ? &store.get(index) : nullptr;
}


// Get tuple of pointers to index-th components, one from each store in the
// given store tuple. If a store does not have component at index-th position,
// nullptr is returned in its place.
namespace help {
template<typename T>
struct GetPtrs;

template<typename... Ts>
struct GetPtrs<std::tuple<Ts...>> {
  auto operator () ( const detail::ComponentStoreRefs<std::tuple<Ts...>>& stores
                   , size_t index) const
  {
    (void) index; // supress unused warning
    return std::make_tuple(get_ptr( std::get<ComponentStore<Ts>&>(stores)
                                  , index)...);
  }
};
}

template<typename T>
auto get_ptrs(const ComponentStoreRefs<T>& stores, size_t index) {
  return help::GetPtrs<T>()(stores, index);
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


// Call entity.components, but use the types from the tuple T
namespace help {
template<typename T>
struct Components;

template<typename... Ts>
struct Components<std::tuple<Ts...>> {
  auto operator () (const Entity& entity) const {
    return entity.components<Ts...>();
  }
};
}

template<typename T>
auto components(const Entity& entity) {
  return help::Components<T>()(entity);
}

} // namespace detail

// Base EntityView
template<typename Need, typename Skip, typename Load>
class EntityView {
public:
  class Iterator;

  class Cursor {
  public:
    const Entity& entity() const {
      return _entity;
    }

    template<typename T>
    typename std::enable_if<!std::is_pointer<T>::value, T&>::type
    get() const {
      static_assert(detail::Contains<Need, T>, "T is not among needed Component types");
      return std::get<T&>(_need);
    }

    template<typename T>
    typename std::enable_if<std::is_pointer<T>::value, T>::type
    get() const {
      static_assert( detail::Contains<Load, typename std::remove_pointer<T>::type>
                   , "T is not among loaded Component types");
      return std::get<T>(_load);
    }

    auto all() const {
      return detail::components<Need>(_entity);
    }

  private:
    Cursor( Entity                      entity
          , const detail::ToRefs<Need>& need
          , const detail::ToPtrs<Load>& load)
      : _entity(entity)
      , _need(need)
      , _load(load)
    {}

  private:
    const Entity               _entity;
    const detail::ToRefs<Need> _need;
    const detail::ToPtrs<Load> _load;

    friend class Iterator;
  };

  class Iterator {
  public:
    bool operator == (Iterator other) const {
      return _index == other._index;
    }

    bool operator != (Iterator other) const {
      return _index != other._index;
    }

    Iterator& operator ++ () {
      advance(1);
      return *this;
    }

    Cursor operator * () const {
      return { _container.get(_index)
             , detail::get_refs<Need>(_need, _index)
             , detail::get_ptrs<Load>(_load, _index) };
    }

  private:
    Iterator( Container&                              container
            , const detail::ComponentStoreRefs<Need>& need
            , const detail::ComponentStoreRefs<Skip>& skip
            , const detail::ComponentStoreRefs<Load>& load
            , size_t                                  index)
      : _container(container)
      , _need(need)
      , _skip(skip)
      , _load(load)
      , _index(index)
    {
      advance(0);
    }

    void advance(size_t offset) {
      _index += offset;

      for (; _index < _container.capacity(); ++_index) {
        if (  _container.contains(_index)
           && detail::has_all <Need>(_need, _index)
           && detail::has_none<Skip>(_skip, _index))
        {
          return;
        }
      }

      _index = _container.capacity();
    }

  private:
    Container&                       _container;
    detail::ComponentStoreRefs<Need> _need;
    detail::ComponentStoreRefs<Skip> _skip;
    detail::ComponentStoreRefs<Load> _load;
    size_t                           _index;

    friend class EntityView;
  };

public:

  EntityView(Container& container)
    : _container(container)
    , _need(detail::slice<Need>(container._stores))
    , _skip(detail::slice<Skip>(container._stores))
    , _load(detail::slice<Load>(container._stores))
  {}

  Iterator begin() {
    return { _container, _need, _skip, _load, 0 };
  }

  Iterator end() {
    return { _container, _need, _skip, _load, _container.capacity() };
  }

  bool empty() {
    return begin() == end();
  }

  auto front() {
    return *begin();
  }

  // Refine this view to include only entities that contain the given Components.
  template<typename... Ts>
  EntityView<std::tuple<Ts...>, Skip, Load> need() const {
    return { _container };
  }

  // Refine this view to include only entities that do not contain any of the
  // given Component.
  template<typename... Ts>
  EntityView<Need, std::tuple<Ts...>, Load> skip() const {
    return { _container };
  }

  // Preload the given components for faster access.
  template<typename... Ts>
  EntityView<Need, Skip, std::tuple<Ts...>> load() const {
    return { _container };
  }

protected:
  Container&                       _container;
  detail::ComponentStoreRefs<Need> _need;
  detail::ComponentStoreRefs<Skip> _skip;
  detail::ComponentStoreRefs<Load> _load;
};

} // namespace secs