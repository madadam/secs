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

TEST_CASE("IsSubset") {
  using secs::detail::IsSubset;

  CHECK((IsSubset<tuple<>, tuple<>>));
  CHECK((IsSubset<tuple<>, tuple<Foo, Bar>>));
  CHECK((IsSubset<tuple<Foo, Bar>, tuple<Foo, Bar, Baz>>));
  CHECK((IsSubset<tuple<Foo, Bar>, tuple<Baz, Bar, Foo>>));

  CHECK(!(IsSubset<tuple<Foo>, tuple<>>));
  CHECK(!(IsSubset<tuple<Foo>, tuple<Bar, Baz>>));
  CHECK(!(IsSubset<tuple<Foo, Bar>, tuple<Bar, Baz>>));
}

TEST_CASE("IsDisjoint") {
  using secs::detail::IsDisjoint;

  CHECK((IsDisjoint<tuple<>, tuple<Foo>>));
  CHECK((IsDisjoint<tuple<Foo>, tuple<>>));
  CHECK((IsDisjoint<tuple<Foo, Bar>, tuple<>>));
  CHECK((IsDisjoint<tuple<Foo>, tuple<Bar>>));
  CHECK((IsDisjoint<tuple<Foo>, tuple<Bar, Baz>>));

  CHECK(!(IsDisjoint<tuple<>, tuple<>>));
  CHECK(!(IsDisjoint<tuple<Foo>, tuple<Foo>>));
  CHECK(!(IsDisjoint<tuple<Foo, Bar>, tuple<Bar, Baz>>));
}