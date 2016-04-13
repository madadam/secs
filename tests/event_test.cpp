#include "catch.hpp"
#include "secs.h"

using namespace secs;

namespace {
struct Position {};

struct ComponentWithImplicitHandlers {
  bool& created;
  bool& destroyed;

  ComponentWithImplicitHandlers(bool& created, bool& destroyed)
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

struct ComponentWithExplicitHandlers {
  size_t& create_count;
  size_t& destroy_count;

  ComponentWithExplicitHandlers( size_t& create_count
                               , size_t& destroy_count)
    : create_count (create_count)
    , destroy_count(destroy_count)
  {}

  void handle(const OnCreate<Position>&) {
    ++create_count;
  }

  void handle(const OnDestroy<Position>&) {
    ++destroy_count;
  }
};

struct TestEvent {};
struct TestEventWithEntity { const Entity entity; };

struct ComponentWithCustomHandlers {
  size_t& count;

  ComponentWithCustomHandlers(size_t& count)
    : count(count)
  {}

  void handle(const TestEventWithEntity&) {
    ++count;
  }
};

} // anonymous namespace

TEST_CASE("Lifetime signals") {
  Container container;

  size_t create_count  = 0;
  size_t destroy_count = 0;

  container.connect<OnCreate<Position>> ([&](auto&) { ++create_count;  });
  container.connect<OnDestroy<Position>>([&](auto&) { ++destroy_count; });

  auto e0 = container.create();
  auto e1 = container.create();
  CHECK(create_count  == 0);
  CHECK(destroy_count == 0);

  e0.create_component<Position>();
  CHECK(create_count  == 1);
  CHECK(destroy_count == 0);

  e1.create_component<Position>();
  CHECK(create_count  == 2);
  CHECK(destroy_count == 0);

  e0.destroy_component<Position>();
  e1.destroy_component<Position>();
  CHECK(create_count  == 2);
  CHECK(destroy_count == 2);
}

TEST_CASE("Implicit lifetime event handlers") {
  Container container;
  auto e = container.create();

  bool created   = false;
  bool destroyed = false;

  e.create_component<ComponentWithImplicitHandlers>(created, destroyed);
  CHECK( created);
  CHECK_FALSE(destroyed);

  e.destroy_component<ComponentWithImplicitHandlers>();
  CHECK(created);
  CHECK(destroyed);
}

TEST_CASE("Explicit lifetime event handlers") {
  Container container;
  container.connect<OnCreate<Position>,  ComponentWithExplicitHandlers>();
  container.connect<OnDestroy<Position>, ComponentWithExplicitHandlers>();

  size_t create_count  = 0;
  size_t destroy_count = 0;

  auto e0 = container.create();
  e0.create_component<ComponentWithExplicitHandlers>( create_count
                                                    , destroy_count);
  CHECK(create_count  == 0);
  CHECK(destroy_count == 0);

  e0.create_component<Position>();
  CHECK(create_count  == 1);
  CHECK(destroy_count == 0);

  e0.destroy_component<Position>();
  CHECK(create_count  == 1);
  CHECK(destroy_count == 1);

  auto e1 = container.create();
  e1.create_component<Position>();
  CHECK(create_count  == 1);
  CHECK(destroy_count == 1);
}

TEST_CASE("Custom signals") {
  Container container;
  size_t count = 0;

  container.connect<TestEvent>([&](auto) { ++count; });
  CHECK(count == 0);

  container.emit(TestEvent{});
  CHECK(count == 1);
}

TEST_CASE("Custom explicit event handlers") {
  Container container;
  container.connect<TestEventWithEntity, ComponentWithCustomHandlers>();

  size_t count0 = 0;
  size_t count1 = 0;

  auto e0 = container.create();
  e0.create_component<ComponentWithCustomHandlers>(count0);

  auto e1 = container.create();
  e1.create_component<ComponentWithCustomHandlers>(count1);

  auto e2 = container.create();

  CHECK(count0 == 0);
  CHECK(count1 == 0);

  container.emit(TestEventWithEntity{ e0 });
  CHECK(count0 == 1);
  CHECK(count1 == 0);

  container.emit(TestEventWithEntity{ e1 });
  CHECK(count0 == 1);
  CHECK(count1 == 1);

  container.emit(TestEventWithEntity{ e2 });
  CHECK(count0 == 1);
  CHECK(count1 == 1);
}
