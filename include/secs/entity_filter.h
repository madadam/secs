#pragma once

#include "secs/detail.h"
#include "secs/filtered_entity.h"

namespace secs {

// Mark component types that are optional.
template<typename> struct Optional {};

// Mark component types that are required (this is the default).
template<typename> struct Required {};

namespace detail {
template<typename T> struct ComponentTypeImpl              { using type = T; };
template<typename T> struct ComponentTypeImpl<Optional<T>> { using type = T; };
template<typename T> struct ComponentTypeImpl<Required<T>> { using type = T; };

template<typename T>
using ComponentType = typename ComponentTypeImpl<T>::type;

template<typename... Ts>
using ComponentStores = std::tuple<ComponentStore<ComponentType<Ts>>*...>;

template<typename R>
using Iterator = decltype(std::begin(std::declval<R>()));

template<typename R>
using Value = decltype(*std::declval<Iterator<R>>());

template<typename T>
constexpr bool IsEntityRange = std::is_same< std::decay_t<Value<T>>
                                           , Entity>::value;

template<typename T> struct SatisfiesOne {
  bool operator () ( const ComponentStore<ComponentType<T>>& store
                   , size_t                                  index) const
  {
    return store.contains(index);
  }
};

template<typename T> struct SatisfiesOne<Optional<T>> {
  bool operator () (const ComponentStore<ComponentType<T>>&, size_t) const {
    return true;
  }
};

template<typename...> struct SatisfiesAll;

template<typename T, typename... Ts> struct SatisfiesAll<T, Ts...> {
  template<typename U>
  bool operator () (const U& stores, size_t index) const {
    using Store = ComponentStore<ComponentType<T>>;
    auto  store = std::get<Store*>(stores);

    return SatisfiesOne<T>()(*store, index)
        && SatisfiesAll<Ts...>()(stores, index);
  }
};

template<> struct SatisfiesAll<> {
  template<typename U>
  bool operator () (const U&, size_t) const {
    return true;
  }
};

template<typename... Ts>
bool satisfies(const ComponentStores<Ts...>& stores, size_t index) {
  return SatisfiesAll<Ts...>()(stores, index);
}

} // namespace detail

template<typename R> Container* get_container(const R&);

template<typename Source, typename... Ts>
class EntityFilter {
  static_assert(detail::IsEntityRange<Source>, "Not an Entity range");

public:
  using value_type = FilteredEntity<detail::ComponentType<Ts>...>;

public:
  class Iterator : public std::iterator<std::forward_iterator_tag, value_type>
  {
  public:
    bool operator == (const Iterator& other) const {
      return _source == other._source;
    }

    bool operator != (const Iterator& other) const {
      return _source != other._source;
    }

    Iterator& operator ++ () {
      advance(1);
      return *this;
    }

    Iterator& operator += (size_t offset) {
      advance(offset);
      return *this;
    }

    auto operator * () const {
      return value_type(*_source, _stores);
    }

  private:
    Iterator( detail::Iterator<Source>              source
            , detail::Iterator<Source>              end
            , const detail::ComponentStores<Ts...>& stores)
      : _source(std::move(source))
      , _end(std::move(end))
      , _stores(stores)
    {
      advance(0);
    }

    void advance(size_t offset) {
      _source += offset;

      for (; _source != _end; ++_source) {
        if (detail::satisfies<Ts...>(_stores, (*_source)._index)) return;
      }
    }

  private:
    detail::Iterator<Source>       _source;
    detail::Iterator<Source>       _end;
    detail::ComponentStores<Ts...> _stores;

    template<typename, typename...>
    friend class EntityFilter;
  };

public:
  EntityFilter(Source source)
    : _source(source)
    , _stores(store_ptrs(source))
  {}

  Iterator begin() const {
    return { std::begin(_source), std::end(_source), _stores };
  }

  Iterator end() const {
    auto e = std::end(_source);
    return { e, e, _stores };
  }

  bool empty() const { return begin() == end(); }
  auto front() const { return *begin(); }

private:
  static detail::ComponentStores<Ts...> store_ptrs(const Source& source) {
    auto container = get_container(source);
    return container
         ? container->template store_ptrs<detail::ComponentType<Ts>...>()
         : detail::ComponentStores<Ts...>();
  }

private:
  Source                         _source;
  detail::ComponentStores<Ts...> _stores;

  friend class Container;
};

template<typename R>
Container* get_container(const R& range) {
  static_assert(detail::IsEntityRange<R>, "Not an Entity range");

  auto first = std::begin(range);

  if (first != std::end(range)) {
    return &(*first).container();
  } else {
    return nullptr;
  }
}

template<typename... Ts>
auto filter(const std::vector<Entity>& entities) {
  return EntityFilter<const std::vector<Entity>&, Ts...>(entities);
}

} // namespace secs
