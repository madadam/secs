#pragma once

#include <type_traits>

namespace secs {

template<typename T>
using Store = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

template<typename T>
size_t store_size = sizeof(Store<T>);

} // namespace secs