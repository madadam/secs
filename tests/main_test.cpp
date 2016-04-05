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

template<typename... Ts>
void unused(Ts...) {}

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

TEST_CASE("Enumerate Entities in Container") {
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

  SECTION("required") {
    counter = count(container.entities<Position>());
    CHECK(counter == 0);

    auto e0 = container.create();
    e0.create_component<Position>(123, 456);
    counter = count(container.entities<Position>());
    CHECK(counter == 1);

    auto e1 = container.create();
    e1.create_component<Velocity>();
    counter = count(container.entities<Position>());
    CHECK(counter == 1);
  }

  SECTION("optional") {
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

    for (auto e : container.entities<Optional<Position>, Optional<Velocity>>()) {
      ++counter;
      if (e.component<Position>()) ++ps;
      if (e.component<Velocity>()) ++vs;
    }

    CHECK(counter == 3);
    CHECK(ps == 2);
    CHECK(vs == 1);
  }

  SECTION("required + optional") {
    auto e0 = container.create();
    e0.create_component<Position>(0, 0);
    e0.create_component<Velocity>();

    auto e1 = container.create();
    e1.create_component<Position>(1, 0);

    counter = count(container.entities<Position, Optional<Velocity>>());
    CHECK(counter == 2);
  }
}

TEST_CASE("Enumerate Entities in vector") {
  Container container;
  auto counter = 0;

  auto e0 = container.create();
  e0.create_component<Position>(0, 0);
  e0.create_component<Velocity>();

  auto e1 = container.create();
  e1.create_component<Position>(0, 0);

  std::vector<Entity> es{ e0, e1 };

  counter = count(filter<Position>(es));
  CHECK(counter == 2);

  counter = count(filter<Position, Velocity>(es));
  CHECK(counter == 1);
}

TEST_CASE("Entity iterators traits") {
  Container container;

  using I0 = decltype(container.entities().begin());
  CHECK((std::is_same< std::iterator_traits<I0>::difference_type
                     , ptrdiff_t>::value));
  CHECK((std::is_same< std::iterator_traits<I0>::value_type
                     , FilteredEntity<>>::value));

  using I1 = decltype(container.entities<Optional<Position>>().begin());
  CHECK((std::is_same< std::iterator_traits<I1>::difference_type
                     , ptrdiff_t>::value));
  CHECK((std::is_same< std::iterator_traits<I1>::value_type
                     , FilteredEntity<Position>>::value));

  using I2 = decltype(container.entities<Position>().begin());
  CHECK((std::is_same< std::iterator_traits<I2>::difference_type
                     , ptrdiff_t>::value));
  CHECK((std::is_same< std::iterator_traits<I2>::value_type
                     , FilteredEntity<Position>>::value));
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

  for (auto e : container.entities<Name>()) {
    auto name = e.component<Name>()->name;
  }
}

////////////////////////////////////////////////////////////////////////////////
template<typename T>
struct TestReceiver : public Receiver<AfterCreate<T>>
                    , public Receiver<AfterDestroy<T>>
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
  TestReceiver<Position> subscriber;

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

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("FilteredEntity conversions") {
  Container container;
  container.create();
  auto e0 = container.entities<Optional<Position>>().front();

  // Entity can be constructed/assigned from FilteredEntity<T...>
  Entity e1(e0);
  Entity e2 = e0;
  Entity e3;
  e3 = e0;

  unused(e1, e2, e3);
}

