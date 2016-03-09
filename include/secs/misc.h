#pragma once

#include <tuple>

namespace secs {
namespace detail {
template<int I, typename... Ts>
struct all {
  template<typename F>
  bool operator () (const std::tuple<Ts...>& tuple, F&& pred) const {
    return pred(std::get<I - 1>(tuple))
        && all<I - 1, Ts...>()(tuple, std::forward<F>(pred));
  }
};

template<typename... Ts>
struct all<0, Ts...> {
  template<typename F>
  bool operator () (const std::tuple<Ts...>&, F&&) const {
    return true;
  }
};
} // namespace detail

// Return true if the predicate returns true for all elements of the tuple.
template<typename... Ts, typename F>
bool all(const std::tuple<Ts...>& tuple, F&& pred) {
  return detail::all<sizeof...(Ts), Ts...>()(tuple, std::forward<F>(pred));
}

namespace detail {
  template<typename F, typename... Args>
  decltype(std::declval<F>()(std::declval<Args>()...), std::true_type())
  is_callable(int);

  template<typename F, typename... Args>
  std::false_type
  is_callable(...);
} // namespace detail

// Test if F is callable given Args... as argments
template<typename F, typename... Args>
constexpr bool is_callable = decltype(detail::is_callable<F, Args...>(0))::value;

} // namespace secs