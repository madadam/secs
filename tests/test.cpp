#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "secs/container.h"
#include "secs/type_index.h"

using namespace secs;

struct Instrument {
  bool visited = false;
};

struct Position {};
struct Velocity {};

struct Name {
public:
  Name(std::string name = {}) : name(std::move(name)) {}
  std::string name;
};

TEST_CASE("Create and destroy Entity") {
  Container container;
  CHECK(container.num_entities() == 0);

  auto e0 = container.create_entity();
  CHECK(container.num_entities() == 1);

  auto e1 = container.create_entity();
  CHECK(container.num_entities() == 2);

  e0.destroy();
  CHECK(container.num_entities() == 1);

  e1.destroy();
  CHECK(container.num_entities() == 0);
}

TEST_CASE("Create Components") {
  Container container;

  SECTION(".component().create() and .component().or_create()") {
    auto e = container.create_entity();
    e.component<Position>().create();
    CHECK(e.component<Position>());

    auto& c0 = *e.component<Velocity>().or_create();
    auto& c1 = *e.component<Velocity>().or_create();
    CHECK(&c0 == &c1);
  }

  SECTION(".add_components() with types") {
    auto e = container.create_entity();
    e.add_components<Position, Velocity>();
    CHECK(e.component<Position>());
    CHECK(e.component<Velocity>());
  }

  SECTION(".add_components() with values") {
    Position c0;
    Velocity c1;
    auto e = container.create_entity();
    e.add_components(c0, c1);
    CHECK(e.component<Position>());
    CHECK(e.component<Velocity>());
  }

  SECTION("create Entity with Component types") {
    auto e = container.create_entity<Position, Velocity>();
    CHECK(e.component<Position>());
    CHECK(e.component<Velocity>());
  }

  SECTION("create Entity with Component values") {
    Position c0;
    Velocity c1;
    auto e = container.create_entity(c0, c1);
    CHECK(e.component<Position>());
    CHECK(e.component<Velocity>());
  }
}

TEST_CASE("Get Entities") {
  Container container;

  auto b = container.entities<Instrument, Position>().empty();
  CHECK(b);

  auto e0 = container.create_entity<Instrument, Position, Velocity>();
  auto e1 = container.create_entity<Instrument, Position>();

  auto b0 = container.entities<>().empty();
  auto b1 = container.entities<Instrument>().empty();
  auto b2 = container.entities<Instrument, Position>().empty();
  auto b3 = container.entities<Instrument, Position, Velocity>().empty();
  CHECK(!b0);
  CHECK(!b1);
  CHECK(!b2);
  CHECK(!b3);

  for (auto e : container.entities<Instrument, Velocity>()) {
    e.component<Instrument>()->visited = true;
  }

  CHECK(e0.component<Instrument>()->visited);
  CHECK(!e1.component<Instrument>()->visited);
}

TEST_CASE("TypeIndex") {
  TypeIndex ti;

  CHECK(ti.get<Position>() == ti.get<Position>());
  CHECK(ti.get<Position>() != ti.get<Velocity>());
}

TEST_CASE("Rich components") {
  Container container;

  auto e0 = container.create_entity();
  auto e1 = container.create_entity();

  e0.component<Name>().create("foo");
  e1.component<Name>().create("bar");

  for (auto e : container.entities<Name>()) {
    auto name = e.component<Name>()->name;
  }
}

TEST_CASE("Copy component") {
  Container container0;

  SECTION("between Entities in the same Container") {
    auto e0 = container0.create_entity<Position>();
    auto e1 = container0.create_entity<Position, Velocity>();

    e1.component<Velocity>().copy_to(e0);

    CHECK(e0.component<Velocity>());
    CHECK(e1.component<Velocity>());
  }

  SECTION("between Entities in different Containers") {
    Container container1;
    auto e0 = container0.create_entity<Position>();
    auto e1 = container1.create_entity<Position, Velocity>();

    e1.component<Velocity>().copy_to(e0);

    CHECK(e0.component<Velocity>());
    CHECK(e1.component<Velocity>());
  }
}

TEST_CASE("Move component") {
  Container container0;

  SECTION("between Entities in the same Container") {
    auto e0 = container0.create_entity<Position>();
    auto e1 = container0.create_entity<Position, Velocity>();

    e1.component<Velocity>().move_to(e0);

    CHECK( e0.component<Velocity>());
    CHECK(!e1.component<Velocity>());
  }

  SECTION("between Entities in different Containers") {
    Container container1;
    auto e0 = container0.create_entity<Position>();
    auto e1 = container1.create_entity<Position, Velocity>();

    e1.component<Velocity>().move_to(e0);

    CHECK( e0.component<Velocity>());
    CHECK(!e1.component<Velocity>());
  }
}

TEST_CASE("Destroy component") {
  Container container;
  auto e = container.create_entity<Position, Velocity>();

  e.component<Velocity>().destroy();

  CHECK(!e.component<Velocity>());
  CHECK( e.component<Position>());
}

