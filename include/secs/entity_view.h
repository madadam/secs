#pragma once

#include <tuple>
#include "secs/container.h"
#include "secs/detail.h"

namespace secs {
namespace detail {
template<typename... Ts>
using ComponentStores = std::tuple<ComponentStore<Ts>*...>;

template<typename R>
using Iterator = decltype(std::begin(std::declval<R>()));

template<typename R>
using Value = decltype(*std::declval<Iterator<R>>());

namespace help {
template<typename T> struct IsEntity {
  static const bool value = false;
};

template<> struct IsEntity<Entity> {
  static const bool value = true;
};

template<typename... Ts> struct IsEntity<LoadedEntity<Ts...>> {
  static const bool value = true;
};
} // namespace help

template<typename T>
constexpr bool IsEntity = help::IsEntity<T>::value;

template<typename T>
constexpr bool IsEntityRange = IsEntity<std::decay_t<Value<T>>>;

} // namespace detail

template<typename...> class LoadedEntity;

template<typename, typename...> class LoadEntityView;
template<typename, typename...> class NeedEntityView;
template<typename, typename...> class SkipEntityView;

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

// TODO: specialize get_container for ContainerEntityView and others

////////////////////////////////////////////////////////////////////////////////
template<typename Base>
class EntityViewExtensions {
public:
  template<typename... Ts>
  LoadEntityView<Base, Ts...> load() const {
    return { *static_cast<const Base*>(this) };
  }

  template<typename... Ts>
  NeedEntityView<LoadEntityView<Base, Ts...>, Ts...> need() const {
    return { load<Ts...>() };
  }

  template<typename... Ts>
  SkipEntityView<LoadEntityView<Base, Ts...>, Ts...> skip() const {
    return { load<Ts...>() };
  }
};

////////////////////////////////////////////////////////////////////////////////
// View of Entities with preloaded Components
template<typename Source, typename... Ts>
class LoadEntityView
  : public EntityViewExtensions<LoadEntityView<Source, Ts...>>
{
  static_assert(detail::IsEntityRange<Source>, "Source is not Entity range");
  // TODO: static assert that Source's loaded types and Ts are disjoint

public:
  class Iterator {
  public:
    bool operator == (const Iterator& other) const {
      return _source == other._source;
    }

    bool operator != (const Iterator& other) const {
      return _source != other._source;
    }

    Iterator& operator ++ () {
      ++_source;
      return *this;
    }

    Iterator& operator += (size_t offset) {
      _source += offset;
      return *this;
    }

    auto operator * () const {
      return (*_source).load(_stores);
    }

  private:
    Iterator( detail::Iterator<Source>              source
            , const detail::ComponentStores<Ts...>& stores)
      : _source(std::move(source))
      , _stores(stores)
    {}

  private:
    detail::Iterator<Source>       _source;
    detail::ComponentStores<Ts...> _stores;

    template<typename, typename...>
    friend class LoadEntityView;
  };

public:
  LoadEntityView(Source source)
    : _source(source)
    , _stores(store_ptrs(source))
  {}

  Iterator begin() const { return { std::begin(_source), _stores }; }
  Iterator end()   const { return { std::end  (_source), _stores }; }

  bool empty() const { return _source.empty(); }
  auto front() const { return *begin(); }

private:
  static detail::ComponentStores<Ts...> store_ptrs(const Source& source) {
    auto container = get_container(source);
    return container ? container->template store_ptrs<Ts...>()
                     : detail::ComponentStores<Ts...>();
  }

private:
  Source                         _source;
  detail::ComponentStores<Ts...> _stores;

  friend Container* get_container(const LoadEntityView<Source, Ts...>& r) {
    return get_container(r._source);
  }
};

////////////////////////////////////////////////////////////////////////////////
// Base class for filtered view of Entities
template<typename Base, typename Source>
class FilteredEntityView : public EntityViewExtensions<Base> {
  static_assert(detail::IsEntityRange<Source>, "Source is not Entity range");

public:
  class Iterator {
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

public:
  FilteredEntityView(Source source)
    : _source(source)
  {}

  Iterator begin() const {
    return { std::begin(_source), std::end(_source) };
  }

  Iterator end() const {
    auto e = std::end(_source);
    return { e, e };
  }

  bool empty() const { return begin() == end(); }
  auto front() const { return *begin(); }

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

  static bool filter(const detail::Value<Source>& e) {
    return e.template contains_all<Ts...>();
  }

  friend Container* get_container(const NeedEntityView<Source, Ts...>& r) {
    return get_container(r._source);
  }
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

  static bool filter(const detail::Value<Source>& e) {
    return e.template contains_none<Ts...>();
  }

  friend Container* get_container(const SkipEntityView<Source, Ts...>& r) {
    return get_container(r._source);
  }
};

template<typename Source>
LoadEntityView<Source> make_entity_view(Source&& source) {
  return { source };
}

} // namespace secs
