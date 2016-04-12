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

TEST_CASE("Enumerate Entities using each") {
  Container container;

  auto e0 = container.create();
  e0.create_component<Position>(0, 0);
  e0.create_component<Velocity>();

  auto e1 = container.create();
  e1.create_component<Position>(0, 0);

  auto counter = 0;
  container.entities<Position>().each([&](auto&) {
    ++counter;
  });
  CHECK(counter == 2);

  counter = 0;
  container.entities<Position, Velocity>().each([&](auto&, auto&) {
    ++counter;
  });
  CHECK(counter == 1);

  counter = 0;
  auto counter_existing = 0;
  container.entities<Position, Optional<Velocity>>().each([&](auto&, auto v) {
    ++counter;
    if (v) ++counter_existing;
  });
  CHECK(counter == 2);
  CHECK(counter_existing == 1);

  counter = 0;
  container.entities<Position, Velocity>().each([&](auto& entity, auto&, auto&) {
    if (entity == e0) ++counter;
  });
  CHECK(counter == 1);
}

TEST_CASE("Create Components") {
  Container container;
  auto e = container.create();
  e.create_component<Position>();
  CHECK(e.component<Position>());
}

TEST_CASE("Ensure Component") {
  Container container;
  auto e = container.create();
  auto c0 = e.ensure_component<Velocity>();
  auto c1 = e.ensure_component<Velocity>();
  CHECK(c0 == c1);
}

////////////////////////////////////////////////////////////////////////////////
struct CopyAssignableComponent {
  int value = 0;
  bool assigned = false;

  CopyAssignableComponent(int value) : value(value) {}
  CopyAssignableComponent(const CopyAssignableComponent&) = delete;
  CopyAssignableComponent(CopyAssignableComponent&&) = default;

  CopyAssignableComponent& operator = (const CopyAssignableComponent& other) {
    value = other.value;
    assigned = true;
    return *this;
  }
  CopyAssignableComponent& operator = (CopyAssignableComponent&&) = delete;
};

struct MoveAssignableComponent {
  int value = 0;
  bool assigned = false;

  MoveAssignableComponent(int value) : value(value) {}
  MoveAssignableComponent(const MoveAssignableComponent&) = delete;
  MoveAssignableComponent(MoveAssignableComponent&&) = default;

  MoveAssignableComponent& operator = (const MoveAssignableComponent&) = delete;
  MoveAssignableComponent& operator = (MoveAssignableComponent&& other) {
    value = other.value;
    assigned = true;
    return *this;
  }
};

struct NonAssignableComponent {
  int value = 0;
  NonAssignableComponent(int value) : value(value) {}
  NonAssignableComponent(NonAssignableComponent&&) = default;

  NonAssignableComponent& operator = (const NonAssignableComponent&) = delete;
  NonAssignableComponent& operator = (NonAssignableComponent&&) = delete;
};

TEST_CASE("Replace Component") {
  Container container;

  auto e = container.create();
  auto c00 = e.create_component<NonAssignableComponent>(123);
  auto c01 = e.create_component<NonAssignableComponent>(456);
  CHECK(c00 == c01);
  CHECK(c00->value == 456);

  auto c10 = e.create_component<CopyAssignableComponent>(123);
  auto c11 = e.create_component<CopyAssignableComponent>(456);
  CHECK(c10 == c11);
  CHECK(c10->value == 456);
  CHECK(c10->assigned);

  auto c20 = e.create_component<MoveAssignableComponent>(123);
  auto c21 = e.create_component<MoveAssignableComponent>(456);
  CHECK(c20 == c21);
  CHECK(c20->value == 456);
  CHECK(c20->assigned);
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

  SECTION("destroy is idempotent") {
    e.destroy_component<Position>();
    CHECK(!e.component<Position>());
    e.destroy_component<Position>();
    CHECK(!e.component<Position>());
  }
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
struct ComponentWithCallbacks {
  bool& created;
  bool& destroyed;

  ComponentWithCallbacks(bool& created, bool& destroyed)
    : created(created)
    , destroyed(destroyed)
  {}

  void on_create(const Entity&) {
    created = true;
  }

  void on_destroy(const Entity&) {
    destroyed = true;
  }
};

TEST_CASE("Lifetime callbacks") {
  Container container;
  auto e = container.create();

  bool created   = false;
  bool destroyed = false;

  e.create_component<ComponentWithCallbacks>(created, destroyed);
  CHECK( created);
  CHECK(!destroyed);

  e.destroy_component<ComponentWithCallbacks>();
  CHECK(created);
  CHECK(destroyed);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Lifetime signals") {
  Container container;

  SECTION("per Container") {
    size_t create_count  = 0;
    size_t destroy_count = 0;

    container.connect<OnCreate<Position>> ([&](auto&) { ++create_count;  });
    container.connect<OnDestroy<Position>>([&](auto&) { ++destroy_count; });

    auto e0 = container.create();
    auto e1 = container.create();
    CHECK(create_count  == 0);
    CHECK(destroy_count == 0);

    e0.create_component<Position>(0, 0);
    CHECK(create_count  == 1);
    CHECK(destroy_count == 0);

    e1.create_component<Position>(0, 0);
    CHECK(create_count  == 2);
    CHECK(destroy_count == 0);

    e0.destroy_component<Position>();
    e1.destroy_component<Position>();
    CHECK(create_count  == 2);
    CHECK(destroy_count == 2);
  }

  SECTION("per Entity") {
    size_t e0_create_count  = 0;
    size_t e0_destroy_count = 0;

    size_t e1_create_count  = 0;
    size_t e1_destroy_count = 0;

    auto e0 = container.create();
    auto e1 = container.create();

    e0.connect<OnCreate<Position>> ([&](auto&) { ++e0_create_count;  });
    e0.connect<OnDestroy<Position>>([&](auto&) { ++e0_destroy_count; });
    e1.connect<OnCreate<Position>> ([&](auto&) { ++e1_create_count;  });
    e1.connect<OnDestroy<Position>>([&](auto&) { ++e1_destroy_count; });

    CHECK(e0_create_count  == 0);
    CHECK(e0_destroy_count == 0);
    CHECK(e1_create_count  == 0);
    CHECK(e1_destroy_count == 0);

    e0.create_component<Position>(0, 0);
    CHECK(e0_create_count  == 1);
    CHECK(e0_destroy_count == 0);
    CHECK(e1_create_count  == 0);
    CHECK(e1_destroy_count == 0);

    e1.create_component<Position>(0, 0);
    CHECK(e0_create_count  == 1);
    CHECK(e0_destroy_count == 0);
    CHECK(e1_create_count  == 1);
    CHECK(e1_destroy_count == 0);

    e0.destroy_component<Position>();
    CHECK(e0_create_count  == 1);
    CHECK(e0_destroy_count == 1);
    CHECK(e1_create_count  == 1);
    CHECK(e1_destroy_count == 0);

    e1.destroy_component<Position>();
    CHECK(e0_create_count  == 1);
    CHECK(e0_destroy_count == 1);
    CHECK(e1_create_count  == 1);
    CHECK(e1_destroy_count == 1);
  }
}

TEST_CASE("Per Entity signals are disconnected when Entity is destroyed") {
  Container container;
  size_t count = 0;

  auto e0 = container.create();
  e0.connect<OnCreate<Velocity>>([&](auto&) { ++count; });

  e0.create_component<Velocity>();
  CHECK(count == 1);

  e0.destroy();

  auto e1 = container.create(); // Will be allocated in the same slot as e0.
  e1.create_component<Velocity>();
  CHECK(count == 1);
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
