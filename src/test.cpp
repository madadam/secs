#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "environment.h"
#include "type_index.h"

using namespace secs;

struct Instrument {
  bool visited = false;
};

struct Position {};
struct Velocity {};

template<typename T>
struct Visitor : System<> {
  void update(Environment<>& env) override {
    for (auto e : env.entities<Instrument, T>()) {
      e.template component<Instrument>()->visited = true;
    }
  }
};

TEST_CASE("Various ways to create components") {
  Environment<> env;

  SECTION(".component().create() and .component().or_create()") {
    auto e = env.create_entity();
    e.component<Position>().create();
    CHECK(e.component<Position>());

    auto& c0 = e.component<Velocity>().or_create();
    auto& c1 = e.component<Velocity>().or_create();
    CHECK(&c0 == &c1);
  }

  SECTION(".add_components() with types") {
    auto e = env.create_entity();
    e.add_components<Position, Velocity>();
    CHECK(e.component<Position>());
    CHECK(e.component<Velocity>());
  }

  SECTION(".add_components() with values") {
    Position c0;
    Velocity c1;
    auto e = env.create_entity();
    e.add_components(c0, c1);
    CHECK(e.component<Position>());
    CHECK(e.component<Velocity>());
  }

  SECTION("create entity with component types") {
    auto e = env.create_entity<Position, Velocity>();
    CHECK(e.component<Position>());
    CHECK(e.component<Velocity>());
  }

  SECTION("create entity with component values") {
    Position c0;
    Velocity c1;
    auto e = env.create_entity(c0, c1);
    CHECK(e.component<Position>());
    CHECK(e.component<Velocity>());
  }
}

TEST_CASE("Entity filtering by components") {
  Environment<> env;

  auto e0 = env.create_entity<Instrument, Position, Velocity>();
  auto e1 = env.create_entity<Instrument, Position>();

  env.add_system(Visitor<Velocity>());
  env.update();

  CHECK(e0.component<Instrument>()->visited);
  CHECK(!e1.component<Instrument>()->visited);
}

TEST_CASE("Destroy entity") {
  Environment<> env;
  auto e = env.create_entity<Position>();
  e.destroy();

  CHECK(!e);
}

TEST_CASE("TypeIndex") {
  TypeIndex ti;

  CHECK(ti.get<Position>() == ti.get<Position>());
  CHECK(ti.get<Position>() != ti.get<Velocity>());
}
