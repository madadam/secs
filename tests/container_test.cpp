#include "catch.hpp"
#include "secs.h"

// DEBUG
#include <iostream>

using namespace secs;

struct Position {
  int x;
  int y;

  Position(int x = 0, int y = 0)
    : x(x), y(y)
  {}
};

struct Velocity {};

struct Name {
public:
  Name(std::string name = {}) : name(std::move(name)) {}
  std::string name;
};

template<typename T>
void unused(T) {}

template<typename E>
size_t count(E entities) {
  size_t result = 0;

  for (auto c : entities) {
    unused(c);
    ++result;
  }

  return result;
}

TEST_CASE("Create and destroy Entity") {
  Container container;
  CHECK(container.size() == 0);

  auto e0 = container.create();
  CHECK(e0);
  CHECK(container.size() == 1);

  auto e1 = container.create();
  CHECK(container.size() == 2);

  e0.destroy();
  CHECK(!e0);
  CHECK(container.size() == 1);

  e1.destroy();
  CHECK(container.size() == 0);
}

TEST_CASE("Copy entity") {
  Container container0;
  auto e0 = container0.create();
  e0.create_component<Position>(123, 456);

  CHECK(container0.size() == 1);

  SECTION("to the same Container") {
    auto e1 = e0.copy();

    CHECK(container0.size() == 2);

    CHECK(e0.component<Position>());
    CHECK(e1.component<Position>());
    CHECK(e1.component<Position>()->x == 123);
    CHECK(e1.component<Position>()->y == 456);
  }

  SECTION("to different Container") {
    Container container1;
    auto e1 = e0.copy_to(container1);

    CHECK(e1 != e0);
    CHECK(container0.size() == 1);
    CHECK(container1.size() == 1);

    CHECK(e1.component<Position>());
    CHECK(e1.component<Position>()->x == 123);
    CHECK(e1.component<Position>()->y == 456);
  }
}

TEST_CASE("Iterate over Entities") {
  Container container;
  size_t counter = 0;

  SECTION("all") {
    counter = count(container.entities());
    CHECK(counter == 0);

    auto e0 = container.create();
    counter = count(container.entities());
    CHECK(counter == 1);

    e0.create_component<Position>(123, 456);
    counter = count(container.entities());
    CHECK(counter == 1);
  }

  SECTION("need") {
    counter = count(container.entities().need<Position>());
    CHECK(counter == 0);

    auto e0 = container.create();
    e0.create_component<Position>(123, 456);
    counter = count(container.entities().need<Position>());
    CHECK(counter == 1);

    auto e1 = container.create();
    e1.create_component<Velocity>();
    counter = count(container.entities().need<Position>());
    CHECK(counter == 1);
  }

  SECTION("skip") {
    counter = count(container.entities().skip<Position>());
    CHECK(counter == 0);

    auto e0 = container.create();
    e0.create_component<Position>(123, 456);
    counter = count(container.entities().skip<Position>());
    CHECK(counter == 0);

    auto e1 = container.create();
    e1.create_component<Velocity>();
    counter = count(container.entities().skip<Position>());
    CHECK(counter == 1);
  }

  SECTION("load") {
    auto e0 = container.create();
    e0.create_component<Position>(0, 0);
    e0.create_component<Velocity>();

    auto e1 = container.create();
    e1.create_component<Position>(1, 0);

    auto e2 = container.create();
    e2.create_component<Name>("hello");

    counter = 0;
    size_t ps = 0;
    size_t vs = 0;

    for (auto c : container.entities().load<Position, Velocity>()) {
      ++counter;
      if (c.get<Position*>()) ++ps;
      if (c.get<Velocity*>()) ++vs;
    }

    CHECK(counter == 3);
    CHECK(ps == 2);
    CHECK(vs == 1);
  }

  SECTION("need + skip") {
    auto e0 = container.create();
    e0.create_component<Position>(0, 0);
    e0.create_component<Velocity>();

    auto e1 = container.create();
    e1.create_component<Position>(1, 0);

    counter = count(container.entities().need<Position>().skip<Velocity>());
    CHECK(counter == 1);
  }

}

TEST_CASE("Create Components") {
  Container container;
  auto e = container.create();
  e.create_component<Position>();
  CHECK(e.component<Position>());

  auto c0 = e.create_component_unless_exists<Velocity>();
  auto c1 = e.create_component_unless_exists<Velocity>();
  CHECK(c0 == c1);
}

TEST_CASE("Copy Component in the same Container") {
  Container container;

  auto e0 = container.create();
  auto e1 = container.create();

  auto c0 = e0.create_component<Position>(123, 456);
  e1.create_component<Position>(*c0);

  auto i0 = e0.component<Position>();
  auto i1 = e1.component<Position>();

  CHECK(i0);
  CHECK(i1);
  CHECK(i1->x == 123);
  CHECK(i1->y == 456);
}

TEST_CASE("Copy Component between different Containers") {
  Container container0;
  Container container1;

  auto e0 = container0.create();
  auto e1 = container1.create();

  auto c0 = e0.create_component<Position>(123, 456);
  e1.create_component<Position>(*c0);

  auto i0 = e0.component<Position>();
  auto i1 = e1.component<Position>();

  CHECK(i0);
  CHECK(i1);
  CHECK(i1->x == 123);
  CHECK(i1->y == 456);
}

TEST_CASE("Move Component in the same Container") {
  Container container;
  auto e0 = container.create();
  auto e1 = container.create();

  auto c0 = e0.create_component<Position>(123, 456);
  e1.create_component<Position>(std::move(*c0));

  auto i1 = e1.component<Position>();
  CHECK(i1);
  CHECK(i1->x == 123);
  CHECK(i1->y == 456);
}

TEST_CASE("Move Component between different Containers") {
  Container container0;
  Container container1;

  auto e0 = container0.create();
  auto e1 = container1.create();

  auto c0 = e0.create_component<Position>(123, 456);
  e1.create_component<Position>(std::move(*c0));

  auto i1 = e1.component<Position>();
  CHECK(i1);
  CHECK(i1->x == 123);
  CHECK(i1->y == 456);
}

////////////////////////////////////////////////////////////////////////////////
struct MovableComponent {
  bool moved = false;

  MovableComponent() = default;

  MovableComponent(const MovableComponent&)
    : moved(false)
  {}

  MovableComponent(MovableComponent&&)
    : moved(true)
  {}
};

TEST_CASE("Moving Component invokes its move constructor") {
  Container container0;
  Container container1;

  auto e0 = container0.create();
  auto e1 = container1.create();

  auto c0 = e0.create_component<MovableComponent>();
  CHECK(!c0->moved);

  auto c1 = e1.create_component<MovableComponent>(std::move(*c0));
  CHECK(c1->moved);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Destroy Component") {
  Container container;
  auto e = container.create();
  e.create_component<Position>();
  e.create_component<Velocity>();

  e.destroy_component<Velocity>();

  CHECK(!e.component<Velocity>());
  CHECK( e.component<Position>());
}

TEST_CASE("Non-POD Components") {
  Container container;

  auto e0 = container.create();
  auto e1 = container.create();

  e0.create_component<Name>("foo");
  e1.create_component<Name>("bar");

  for (auto c : container.entities().need<Name>()) {
    auto name = c.get<Name>().name;
  }
}

TEST_CASE("ComponentView") {
  Container container;

  auto e  = container.create();
  auto c0 = e.create_component<Position>(123, 456);
  auto c1 = e.create_component<Velocity>();

  auto cs0 = e.components<Position, Velocity>();
  CHECK(&cs0.get<Position>() == c0.get());
  CHECK(&cs0.get<Velocity>() == c1.get());

  auto cs1 = make_component_view(c0, c1);
  CHECK(&cs1.get<Position>() == c0.get());
  CHECK(&cs1.get<Velocity>() == c1.get());
}

////////////////////////////////////////////////////////////////////////////////
template<typename T>
struct TestSubscriber : public Subscriber<AfterCreate<T>>
                      , public Subscriber<AfterDestroy<T>>
{
  size_t created   = 0;
  size_t destroyed = 0;

  void receive(const AfterCreate<T>&) override {
    ++created;
  }

  void receive(const AfterDestroy<T>&) override {
    ++destroyed;
  }
};

TEST_CASE("Lifetime events") {
  Container container;
  TestSubscriber<Position> subscriber;

  container.subscribe<AfterCreate<Position>>(subscriber);
  container.subscribe<AfterDestroy<Position>>(subscriber);

  CHECK(subscriber.created   == 0);
  CHECK(subscriber.destroyed == 0);

  auto e = container.create();
  e.create_component<Position>(123, 456);
  CHECK(subscriber.created   == 1);
  CHECK(subscriber.destroyed == 0);

  e.destroy();
  CHECK(subscriber.created   == 1);
  CHECK(subscriber.destroyed == 1);
}
