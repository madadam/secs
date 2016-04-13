#pragma once

namespace secs {

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

} // namespace secs