#include "catch.hpp"
#include "secs/misc.h"

using namespace secs;

namespace {
template<int I>
struct Value {
  int value() const { return I; }
};
}

TEST_CASE("tuple all()") {
  auto t = std::make_tuple(Value<0>(), Value<1>(), Value<2>());
  auto r0 = all(t, [](auto v) { return v.value() > 0; });
  auto r1 = all(t, [](auto v) { return v.value() < 3; });

  CHECK(!r0);
  CHECK( r1);
}