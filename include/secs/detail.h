#pragma once

#include <tuple>

namespace secs {
namespace detail {

// Test if tuple contains the given type.
namespace help {
template<typename T, typename E>
struct Contains;

template<typename T, typename... Ts, typename E>
struct Contains<std::tuple<T, Ts...>, E> {
  static const bool value = std::is_same<T, E>::value
                         || Contains<std::tuple<Ts...>, E>::value;
};

template<typename E>
struct Contains<std::tuple<>, E> {
  static const bool value = false;
};
}

template<typename T, typename E>
constexpr bool Contains = help::Contains<T, E>::value;



// Return true if the predicate returns true for all elements of the tuple.
namespace help {
template<int I, typename... Ts>
struct All {
  template<typename F>
  bool operator () (const std::tuple<Ts...>& tuple, F&& pred) const {
    return pred(std::get<I - 1>(tuple))
        && All<I - 1, Ts...>()(tuple, std::forward<F>(pred));
  }
};

template<typename... Ts>
struct All<0, Ts...> {
  template<typename F>
  bool operator () (const std::tuple<Ts...>&, F&&) const {
    return true;
  }
};
}

template<typename... Ts, typename F>
bool all(const std::tuple<Ts...>& tuple, F&& pred) {
  return help::All<sizeof...(Ts), Ts...>()(tuple, std::forward<F>(pred));
}

}} // namespace secs::detail