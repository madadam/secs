#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "secs/container.h"
#include "secs/entity.h"
#include "secs/type_index.h"

using namespace secs;

struct Instrument {
  bool visited = false;
  int data = 0;
  size_t after_create_count = 0;
  size_t after_copy_count = 0;
  size_t after_move_count = 0;

  Instrument(int data = 0) : data(data) {}

  Instrument(const Instrument& other)
    : visited(other.visited)
    , data(other.data)
    , after_create_count(other.after_create_count)
    , after_copy_count(other.after_copy_count)
    , after_move_count(other.after_move_count)
  {}

  Instrument(Instrument&& other)
    : visited(other.visited)
    , data(other.data)
    , after_create_count(other.after_create_count)
    , after_copy_count(other.after_copy_count)
    , after_move_count(other.after_move_count)
  {}
};

void after_create(ComponentPtr<Instrument> c) {
  ++c->after_create_count;
}

void after_copy(ComponentPtr<Instrument> c) {
  ++c->after_copy_count;
}

void after_move(ComponentPtr<Instrument> c) {
  ++c->after_move_count;
}

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
  auto e0 = container0.create<Instrument>(987654);

  CHECK(container0.size() == 1);

  SECTION("to the same Container") {
    auto e1 = e0.copy();

    CHECK(container0.size() == 2);

    CHECK(e0.component<Instrument>());
    CHECK(e1.component<Instrument>());
    CHECK(e1.component<Instrument>()->data == 987654);
  }

  SECTION("to different Container") {
    Container container1;
    auto e1 = e0.copy_to(container1);

    CHECK(e1 != e0);
    CHECK(container0.size() == 1);
    CHECK(container1.size() == 1);

    CHECK(e1.component<Instrument>());
    CHECK(e1.component<Instrument>()->data == 987654);
  }
}

TEST_CASE("Move entity") {
  Container container0;
  Container container1;
  auto e0 = container0.create<Instrument>(987654);

  CHECK(container0.size() == 1);
  CHECK(container1.size() == 0);

  auto e1 = e0.move_to(container1);

  CHECK(e1 != e0);
  CHECK(!e0);

  CHECK(container0.size() == 0);
  CHECK(container1.size() == 1);

  CHECK(e1.component<Instrument>());
  CHECK(e1.component<Instrument>()->data == 987654);
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

TEST_CASE("Copy Component in the same Container") {
  Container container;

  auto e0 = container.create();
  auto e1 = container.create();

  e0.component<Instrument>().create(123456);
  e1.component<Instrument>().create(*e0.component<Instrument>());

  auto i0 = e0.component<Instrument>();
  auto i1 = e1.component<Instrument>();

  CHECK(i0);
  CHECK(i1);
  CHECK(i1->data == 123456);
}

TEST_CASE("Copy Component between different Containers") {
  Container container0;
  Container container1;

  auto e0 = container0.create();
  auto e1 = container1.create();

  e0.component<Instrument>().create(123456);
  e1.component<Instrument>().create(*e0.component<Instrument>());

  auto i0 = e0.component<Instrument>();
  auto i1 = e1.component<Instrument>();

  CHECK(i0);
  CHECK(i1);
  CHECK(i1->data == 123456);
}

TEST_CASE("Move Component in the same Container") {
  Container container;
  auto e0 = container.create();
  auto e1 = container.create();

  e0.component<Instrument>().create(123456);
  e1.component<Instrument>().create(std::move(*e0.component<Instrument>()));

  auto i1 = e1.component<Instrument>();
  CHECK(i1);
  CHECK(i1->data == 123456);
}

TEST_CASE("Move Component between different Containers") {
  Container container0;
  Container container1;

  auto e0 = container0.create();
  auto e1 = container1.create();

  e0.component<Instrument>().create(123456);
  e1.component<Instrument>().create(std::move(*e0.component<Instrument>()));

  auto i1 = e1.component<Instrument>();
  CHECK(i1);
  CHECK(i1->data == 123456);
}

TEST_CASE("Destroy Component") {
  Container container;
  auto e = container.create<Position, Velocity>();

  e.component<Velocity>().destroy();

  CHECK(!e.component<Velocity>());
  CHECK( e.component<Position>());
}

TEST_CASE("Non-POD Components") {
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

