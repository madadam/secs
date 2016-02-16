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

TEST_CASE("Create and destroy Entity") {
  Environment<> env;
  CHECK(env.num_entities() == 0);

  auto e0 = env.create_entity();
  CHECK(env.num_entities() == 1);

  auto e1 = env.create_entity();
  CHECK(env.num_entities() == 2);

  e0.destroy();
  CHECK(env.num_entities() == 1);

  e1.destroy();
  CHECK(env.num_entities() == 0);
}

TEST_CASE("Create Components") {
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

  SECTION("create Entity with Component types") {
    auto e = env.create_entity<Position, Velocity>();
    CHECK(e.component<Position>());
    CHECK(e.component<Velocity>());
  }

  SECTION("create Entity with Component values") {
    Position c0;
    Velocity c1;
    auto e = env.create_entity(c0, c1);
    CHECK(e.component<Position>());
    CHECK(e.component<Velocity>());
  }
}

TEST_CASE("Entity filtering by Components") {
  Environment<> env;

  auto e0 = env.create_entity<Instrument, Position, Velocity>();
  auto e1 = env.create_entity<Instrument, Position>();

  env.add_system(Visitor<Velocity>());
  env.update();

  CHECK(e0.component<Instrument>()->visited);
  CHECK(!e1.component<Instrument>()->visited);
}

TEST_CASE("TypeIndex") {
  TypeIndex ti;

  CHECK(ti.get<Position>() == ti.get<Position>());
  CHECK(ti.get<Position>() != ti.get<Velocity>());
}
