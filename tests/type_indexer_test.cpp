#include "catch.hpp"
#include "secs/type_indexer.h"

using namespace secs;

namespace {
struct Position {};
struct Velocity {};
} // anonymous namespace

TEST_CASE("TypeIndexer") {
  TypeIndexer ti;

  CHECK(ti.get<Position>() == ti.get<Position>());
  CHECK(ti.get<Position>() != ti.get<Velocity>());
}

