#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "environment.h"
#include "type_index.h"

using namespace secs;

struct Instrument {
  bool* visited;
};

struct Position {};
struct Velocity {};

template<typename T>
struct Visitor : System<> {
  void update(Environment<>& env) override {
    for (auto e : env.entities<Instrument, T>()) {
      *e.template get_component<Instrument>().visited = true;
    }
  }
};

TEST_CASE("Smoke test") {
  Environment<> env;

  bool e0_visited = false;
  bool e1_visited = false;

  auto e0 = env.create_entity();
  e0.add_component(Instrument{ &e0_visited });
  e0.add_component(Position());
  e0.add_component(Velocity());

  auto e1 = env.create_entity();
  e1.add_component(Instrument{ &e1_visited });
  e1.add_component(Position());

  env.add_system(Visitor<Velocity>());
  env.update();

  CHECK(e0_visited);
  CHECK(!e1_visited);
}

TEST_CASE("Destroy entity") {
  Environment<> env;
  auto e = env.create_entity();
  e.add_component(Position());
  e.destroy();

  CHECK_THROWS_AS(e.get_component<Position>(), error::EntityInvalid);
}

TEST_CASE("TypeIndex") {
  TypeIndex ti;

  CHECK(ti.get<Position>() == ti.get<Position>());
  CHECK(ti.get<Position>() != ti.get<Velocity>());
}
