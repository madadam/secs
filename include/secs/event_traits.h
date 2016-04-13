#pragma once

namespace secs {
namespace detail {

template<typename T>
struct HasEntityFun {
private:
  template<typename U>
  static constexpr decltype(std::declval<U>().entity(), true)
  test(int) { return true; }

  template<typename>
  static constexpr bool test(...) { return false; }
public:
  static const bool value = test<T>(0);
};

template<typename T>
struct HasEntityVar {
private:
  template<typename U>
  static constexpr decltype(std::declval<U>().entity, true)
  test(int) { return true; }

  template<typename>
  static constexpr bool test(...) { return false; }
public:
  static const bool value = test<T>(0);
};

}

// TODO: return true only if both
//    event_entity(E()) and
//    event_handle(T(), E())
// are valid expressions.
template<typename T, typename E>
constexpr bool CanHandleEvent = true;

template< typename E
        , std::enable_if_t<detail::HasEntityFun<E>::value>* = nullptr>
decltype(auto) event_entity(const E& event) {
  return event.entity();
}

template< typename E
        , std::enable_if_t<!detail::HasEntityFun<E>::value &&
                            detail::HasEntityVar<E>::value>* = nullptr>
decltype(auto) event_entity(const E& event) {
  return event.entity;
}

// Handle the Event on the Component. By default calls T::handle(event), but
// can be redefined using ADL.
template<typename T, typename E>
void event_handle(T& component, const E& event) {
  return component.handle(event);
}

} // namespace secs
