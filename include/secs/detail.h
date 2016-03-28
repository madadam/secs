#pragma once

#include <tuple>

namespace secs {
namespace detail {

// Test if type pack Us contains the type T.
namespace help {
template<typename T, typename... Us>
struct Contains;

template<typename T, typename U, typename... Us>
struct Contains<T, U, Us...> {
  static const bool value = std::is_same<T, U>::value
                         || Contains<T, Us...>::value;
};

template<typename T>
struct Contains<T> {
  static const bool value = false;
};
}

template<typename... Ts>
constexpr bool Contains = help::Contains<Ts...>::value;



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