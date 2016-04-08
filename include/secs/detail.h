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

struct IsCallableImpl {
  template<typename F, typename... Args>
  static decltype(std::declval<F>()(std::declval<Args>()...), std::true_type())
  test(int);

  template<typename F, typename... Args>
  static std::false_type
  test(...);
};

// Test if F can be called with arguments Args...
template<typename F, typename... Args>
constexpr bool IsCallable = decltype(IsCallableImpl::test<F, Args...>(0))::value;

}} // namespace secs::detail