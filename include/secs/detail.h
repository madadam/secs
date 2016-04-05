#pragma once

#include <tuple>

namespace secs {
namespace detail {

// Contains implementation
template<typename T, typename... Us>
struct ContainsImpl;

template<typename T, typename U, typename... Us>
struct ContainsImpl<T, U, Us...> {
  static const bool value = std::is_same<T, U>::value
                         || ContainsImpl<T, Us...>::value;
};

template<typename T>
struct ContainsImpl<T> {
  static const bool value = false;
};

// Test if type pack Us contains the type T.
template<typename T, typename... Us>
constexpr bool Contains = ContainsImpl<T, Us...>::value;

// Test if the tuple contains the type T.
template<typename T, typename... Us>
constexpr bool Contains<T, std::tuple<Us...>> = ContainsImpl<T, Us...>::value;

}} // namespace secs::detail