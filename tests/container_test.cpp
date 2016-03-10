#include "catch.hpp"
#include "secs/container.h"
#include "secs/entity.h"
#include "secs/lifetime_subscriber.h"

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

struct EntityAwareComponent {
  Entity self = nullptr;
};

void on_create(Entity e, ComponentPtr<EntityAwareComponent> c) {
  c->self = e;
}

class PositionSubscriber : public LifetimeSubscriber<Position> {
public:
  int creates = 0;
  int destroys = 0;

  void on_create(Entity, ComponentPtr<Position>) override {
    ++creates;
  }

  void on_destroy(Entity, ComponentPtr<Position>) override {
    ++destroys;
  }
};

template<typename T>
void unused(T) {}

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

TEST_CASE("Move entity") {
  Container container0;
  Container container1;
  auto e0 = container0.create();
  e0.create_component<Position>(123, 456);

  CHECK(container0.size() == 1);
  CHECK(container1.size() == 0);

  auto e1 = e0.move_to(container1);

  CHECK(e1 != e0);
  CHECK(!e0);

  CHECK(container0.size() == 0);
  CHECK(container1.size() == 1);

  CHECK(e1.component<Position>());
  CHECK(e1.component<Position>()->x == 123);
  CHECK(e1.component<Position>()->y == 456);
}

TEST_CASE("Iterate over entities") {
  Container container;
  size_t counter = 0;

  for (auto c : container.entities<>()) {
    unused(c);
    ++counter;
  }
  CHECK(counter == 0);

  auto e0 = container.create();
  e0.create_component<Position>(123, 456);

  counter = 0;
  for (auto c : container.entities<>()) {
    unused(c);
    ++counter;
  }
  CHECK(counter == 1);

  counter = 0;
  for (auto c : container.entities<Position>()) {
    unused(c);
    ++counter;
  }
  CHECK(counter == 1);

  counter = 0;
  for (auto c : container.entities<Position>()) {
    if (c.entity() == e0) ++counter;
  }
  CHECK(counter == 1);

  counter = 0;
  for (auto c : container.entities<Position>()) {
    if (c.get<Position>().x == 123 && c.get<Position>().y == 456) {
      ++counter;
    }
  }
  CHECK(counter == 1);

  for (auto c : container.entities<Position>()) {
    c.get<Position>().x = 789;
  }
  CHECK(e0.component<Position>()->x == 789);

  auto e1 = container.create();
  e1.create_component<Position>();
  e1.create_component<Velocity>();

  counter = 0;
  for (auto c : container.entities<Position>()) {
    unused(c);
    ++counter;
  }
  CHECK(counter == 2);

  counter = 0;
  for (auto c : container.entities<Position, Velocity>()) {
    unused(c);
    ++counter;
  }
  CHECK(counter == 1);

}

TEST_CASE("Create Components") {
  Container container;
  auto e = container.create();
  e.create_component<Position>();
  CHECK(e.component<Position>());

  auto c0 = e.ensure_component<Velocity>();
  auto c1 = e.ensure_component<Velocity>();
  CHECK(c0 == c1);
}

TEST_CASE("Copy Component in the same Container") {
  Container container;

  auto e0 = container.create();
  auto e1 = container.create();

  e0.create_component<Position>(123, 456);
  e1.create_component<Position>(*e0.component<Position>());

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

  e0.create_component<Position>(123, 456);
  e1.create_component<Position>(*e0.component<Position>());

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

  e0.create_component<Position>(123, 456);
  e1.create_component<Position>(std::move(*e0.component<Position>()));

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

  e0.create_component<Position>(123, 456);
  e1.create_component<Position>(std::move(*e0.component<Position>()));

  auto i1 = e1.component<Position>();
  CHECK(i1);
  CHECK(i1->x == 123);
  CHECK(i1->y == 456);
}

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

  for (auto c : container.entities<Name>()) {
    auto name = c.get<Name>().name;
  }
}

TEST_CASE("Lifetime events") {
  Container container;
  PositionSubscriber subscriber;

  container.subscribe<Position>(subscriber);

  CHECK(subscriber.creates  == 0);
  CHECK(subscriber.destroys == 0);

  auto e0 = container.create();
  CHECK(subscriber.creates  == 0);
  CHECK(subscriber.destroys == 0);

  e0.create_component<Position>();
  CHECK(subscriber.creates  == 1);
  CHECK(subscriber.destroys == 0);

  auto e1 = e0.copy();
  CHECK(subscriber.creates  == 2);
  CHECK(subscriber.destroys == 0);

  e1.destroy_component<Position>();
  CHECK(subscriber.creates  == 2);
  CHECK(subscriber.destroys == 1);

  e0.destroy();
  CHECK(subscriber.creates  == 2);
  CHECK(subscriber.destroys == 2);
}

TEST_CASE("Lifetime events on move") {
  Container container0;
  PositionSubscriber subscriber0;
  container0.subscribe<Position>(subscriber0);

  Container container1;
  PositionSubscriber subscriber1;
  container1.subscribe<Position>(subscriber1);

  auto e0 = container0.create();
  e0.create_component<Position>();
  CHECK(subscriber0.creates  == 1);
  CHECK(subscriber0.destroys == 0);
  CHECK(subscriber1.creates  == 0);
  CHECK(subscriber1.destroys == 0);

  e0.move_to(container1);
  CHECK(subscriber0.creates  == 1);
  CHECK(subscriber0.destroys == 1);
  CHECK(subscriber1.creates  == 1);
  CHECK(subscriber1.destroys == 0);
}

TEST_CASE("Lifetime callbacks") {
  Container container;
  auto e = container.create();
  auto c = e.create_component<EntityAwareComponent>();

  CHECK(c->self == e);
}
