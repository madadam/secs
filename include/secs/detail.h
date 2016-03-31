#pragma once

#include <tuple>

namespace secs {
namespace detail {

namespace help {

// Contains implementation
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

// IsSubset implementation
template<typename...>
struct IsSubset;

template<typename T, typename... Ts, typename... Us>
struct IsSubset<std::tuple<T, Ts...>, std::tuple<Us...>> {
  static const bool value =
       Contains<T, Us...>::value
    && IsSubset<std::tuple<Ts...>, std::tuple<Us...>>::value;
};

template<typename... Us>
struct IsSubset<std::tuple<>, std::tuple<Us...>> {
  static const bool value = true;
};

// IsDisjoint implementation
template<typename...>
struct IsDisjoint;

template<typename T, typename... Ts, typename... Us>
struct IsDisjoint<std::tuple<T, Ts...>, std::tuple<Us...>> {
  static const bool value =
      !Contains<T, Us...>::value
    && IsDisjoint<std::tuple<Ts...>, std::tuple<Us...>>::value;
};

template<typename T, typename... Ts>
struct IsDisjoint<std::tuple<T, Ts...>, std::tuple<>> {
  static const bool value = true;
};

template<typename U, typename... Us>
struct IsDisjoint<std::tuple<>, std::tuple<U, Us...>> {
  static const bool value = true;
};

template<>
struct IsDisjoint<std::tuple<>, std::tuple<>> {
  static const bool value = false;
};

}

// Test if type pack Us contains the type T.
template<typename T, typename... Us>
constexpr bool Contains = help::Contains<T, Us...>::value;

// Test if the tuple contains the type T.
template<typename T, typename... Us>
constexpr bool Contains<T, std::tuple<Us...>> = help::Contains<T, Us...>::value;

// Test if all types in the first tuple are also contained in the second one.
template<typename T, typename U>
constexpr bool IsSubset = help::IsSubset<T, U>::value;

// Test if the tuples share no common type.
template<typename T, typename U>
constexpr bool IsDisjoint = help::IsDisjoint<T, U>::value;

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

// Return true if the predicate returns true for all elements of the tuple.
template<typename... Ts, typename F>
bool all(const std::tuple<Ts...>& tuple, F&& pred) {
  return help::All<sizeof...(Ts), Ts...>()(tuple, std::forward<F>(pred));
}

}} // namespace secs::detail