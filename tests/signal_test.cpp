#include "catch.hpp"
#include "secs/signal.h"

using secs::Connection;
using secs::Signal;

namespace {

struct TestEvent {
  int a = 0;
  int b = 0;
  TestEvent() = default;
};

template<typename F>
std::result_of_t<F(int, int)> invoke(F fun, const TestEvent& event) {
  return fun(event.a, event.b);
}

}

TEST_CASE("Signal basics") {
  Signal<TestEvent> signal0;
  bool slot0_called = false;

  signal0.connect([&](auto) { slot0_called = true; });
  signal0(TestEvent{});
  CHECK(slot0_called);

  Signal<TestEvent> signal1;
  int slot1_called_with = 0;

  signal1.connect([&](auto e) { slot1_called_with = e.a; });
  signal1(TestEvent{ 123, 456 });
  CHECK(slot1_called_with == 123);
}

TEST_CASE("Signal disconnection") {
  Signal<TestEvent> signal;
  auto count0 = 0;
  auto count1 = 0;

  SECTION("manual") {
    auto conn0 = signal.connect([&](auto) { ++count0; });
    auto conn1 = signal.connect([&](auto) { ++count1; });
    CHECK(conn0.connected());
    CHECK(conn1.connected());

    signal(TestEvent{});
    CHECK(count0 == 1);
    CHECK(count1 == 1);

    conn0.disconnect();
    CHECK_FALSE(conn0.connected());
    CHECK(conn1.connected());

    signal(TestEvent{});
    CHECK(count0 == 1);
    CHECK(count1 == 2);

    conn1.disconnect();
    CHECK_FALSE(conn0.connected());
    CHECK_FALSE(conn1.connected());

    signal(TestEvent{});
    CHECK(count0 == 1);
    CHECK(count1 == 2);
  }

  SECTION("scoped") {
    {
      auto conn = signal.connect([&](auto) { ++count0; }).scoped();
      signal(TestEvent{});
      CHECK(count0 == 1);
    }

    signal(TestEvent{});
    CHECK(count0 == 1);
  }
}

TEST_CASE("Signal invocation") {
  Signal<TestEvent> signal;
  size_t count = 0;

  signal.connect([&](const TestEvent&) { ++count; });
  signal.connect([&](TestEvent)        { ++count; });
  signal.connect([&](auto)             { ++count; });
  signal.connect([&](auto&)            { ++count; });
  signal.connect([&]()                 { ++count; });
  signal.connect([&](int, int)         { ++count; });

  signal(TestEvent{});

  CHECK(count == 6);
}
