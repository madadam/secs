#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "secs/container.h"
#include "secs/entity.h"
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
  CHECK(container.size() == 0);

  auto e0 = container.create();
  CHECK(container.size() == 1);

  auto e1 = container.create();
  CHECK(container.size() == 2);

  e0.destroy();
  CHECK(container.size() == 1);

  e1.destroy();
  CHECK(container.size() == 0);
}

TEST_CASE("Copy entity") {
  Container container0;
  auto e0 = container0.create<Position, Velocity>();

  CHECK(container0.size() == 1);

  SECTION("to the same Container") {
    auto e1 = e0.copy();

    CHECK(container0.size() == 2);

    CHECK(e0.component<Position>());
    CHECK(e0.component<Velocity>());

    CHECK(e1.component<Position>());
    CHECK(e1.component<Velocity>());
  }

  SECTION("to different Container") {
    Container container1;
    auto e1 = e0.copy_to(container1);

    CHECK(e1 != e0);
    CHECK(container0.size() == 1);
    CHECK(container1.size() == 1);
  }
}

TEST_CASE("Move entity") {
  Container container0;
  Container container1;
  auto e0 = container0.create<Position, Velocity>();

  CHECK(container0.size() == 1);
  CHECK(container1.size() == 0);

  auto e1 = e0.move_to(container1);

  CHECK(e1 != e0);
  CHECK(!e0);

  CHECK(e1.component<Position>());
  CHECK(e1.component<Velocity>());

  CHECK(container0.size() == 0);
  CHECK(container1.size() == 1);
}

TEST_CASE("Get Entities") {
  Container container;

  auto b = container.all<Instrument, Position>().empty();
  CHECK(b);

  auto e0 = container.create<Instrument, Position, Velocity>();
  auto e1 = container.create<Instrument, Position>();

  auto b0 = container.all<>().empty();
  auto b1 = container.all<Instrument>().empty();
  auto b2 = container.all<Instrument, Position>().empty();
  auto b3 = container.all<Instrument, Position, Velocity>().empty();
  CHECK(!b0);
  CHECK(!b1);
  CHECK(!b2);
  CHECK(!b3);

  for (auto e : container.all<Instrument, Velocity>()) {
    e.component<Instrument>()->visited = true;
  }

  CHECK(e0.component<Instrument>()->visited);
  CHECK(!e1.component<Instrument>()->visited);
}

TEST_CASE("Create Components") {
  Container container;

  SECTION(".component().create() and .component().or_create()") {
    auto e = container.create();
    e.component<Position>().create();
    CHECK(e.component<Position>());

    auto& c0 = *e.component<Velocity>().or_create();
    auto& c1 = *e.component<Velocity>().or_create();
    CHECK(&c0 == &c1);
  }

  SECTION("create Entity with Component types") {
    auto e = container.create<Position, Velocity>();
    CHECK(e.component<Position>());
    CHECK(e.component<Velocity>());
  }

  SECTION("create Entity with Component values") {
    Position c0;
    Velocity c1;
    auto e = container.create(c0, c1);
    CHECK(e.component<Position>());
    CHECK(e.component<Velocity>());
  }
}

TEST_CASE("Copy component") {
  Container container0;

  SECTION("between Entities in the same Container") {
    auto e0 = container0.create<Position, Velocity>();
    auto e1 = container0.create<Position>();

    e0.component<Velocity>().copy_to(e1);

    CHECK(e0.component<Velocity>());
    CHECK(e1.component<Velocity>());
  }

  SECTION("between Entities in different Containers") {
    Container container1;
    auto e0 = container1.create<Position, Velocity>();
    auto e1 = container0.create<Position>();

    e0.component<Velocity>().copy_to(e1);

    CHECK(e0.component<Velocity>());
    CHECK(e1.component<Velocity>());
  }
}

TEST_CASE("Move component") {
  Container container0;

  SECTION("between Entities in the same Container") {
    auto e0 = container0.create<Position, Velocity>();
    auto e1 = container0.create<Position>();

    e0.component<Velocity>().move_to(e1);

    CHECK(!e0.component<Velocity>());
    CHECK( e1.component<Velocity>());
  }

  SECTION("between Entities in different Containers") {
    Container container1;
    auto e0 = container1.create<Position, Velocity>();
    auto e1 = container0.create<Position>();

    e0.component<Velocity>().move_to(e1);

    CHECK(!e0.component<Velocity>());
    CHECK( e1.component<Velocity>());
  }
}

TEST_CASE("Destroy component") {
  Container container;
  auto e = container.create<Position, Velocity>();

  e.component<Velocity>().destroy();

  CHECK(!e.component<Velocity>());
  CHECK( e.component<Position>());
}

TEST_CASE("Rich components") {
  Container container;

  auto e0 = container.create();
  auto e1 = container.create();

  e0.component<Name>().create("foo");
  e1.component<Name>().create("bar");

  for (auto e : container.all<Name>()) {
    auto name = e.component<Name>()->name;
  }
}

TEST_CASE("TypeIndex") {
  TypeIndex ti;

  CHECK(ti.get<Position>() == ti.get<Position>());
  CHECK(ti.get<Position>() != ti.get<Velocity>());
}

