#include "catch.hpp"
#include "secs.h"

using namespace secs;

TEST_CASE("Basics") {
  Version v;
  CHECK(!v.exists());
  CHECK(v.serial() == 0);

  v.create();
  CHECK(v.exists());
  CHECK(v.serial() == 1);

  v.destroy();
  CHECK(!v.exists());
  CHECK(v.serial() == 2);

  v.create();
  CHECK(v.exists());
  CHECK(v.serial() == 3);
}

TEST_CASE("Comparison") {
  Version v0;
  Version v1;
  CHECK(v1 == v0);

  v0.create();
  CHECK(v1 < v0);

  v1.create();
  CHECK(v1 == v0);

  v0.destroy();
  CHECK(v1 < v0);
}