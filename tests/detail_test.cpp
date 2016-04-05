#include "catch.hpp"
#include "secs/detail.h"

using std::tuple;

namespace {
struct Foo {};
struct Bar {};
struct Baz {};
} // anonymous namespace

TEST_CASE("Contains") {
  using secs::detail::Contains;

  CHECK((Contains<Foo, Foo>));
  CHECK((Contains<Foo, Foo, Bar>));
  CHECK((Contains<Foo, Bar, Foo>));

  CHECK(!(Contains<Foo>));
  CHECK(!(Contains<Foo, Bar, Baz>));
}
