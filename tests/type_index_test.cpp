#include "catch.hpp"
#include "secs/type_index.h"

using namespace secs;

namespace {
struct Position {};
struct Velocity {};
} // anonymous namespace

TEST_CASE("TypeIndex") {
  TypeIndex ti;

  CHECK(ti.get<Position>() == ti.get<Position>());
  CHECK(ti.get<Position>() != ti.get<Velocity>());
}

