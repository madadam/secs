#include "catch.hpp"
#include "secs/signal.h"

using secs::Connection;
using secs::Signal;

TEST_CASE("Signal basics") {
  Signal<> signal0;
  bool slot0_called = false;

  signal0.connect([&]() { slot0_called = true; });
  signal0();
  CHECK(slot0_called);

  Signal<int> signal1;
  int slot1_called_with = 0;

  signal1.connect([&](int arg) { slot1_called_with = arg; });
  signal1(123);
  CHECK(slot1_called_with == 123);
}

TEST_CASE("Signal disconnection") {
  Signal<> signal;
  auto count0 = 0;
  auto count1 = 0;

  SECTION("Manual") {
    auto conn0 = signal.connect([&]() { ++count0; });
    auto conn1 = signal.connect([&]() { ++count1; });
    CHECK(conn0.connected());
    CHECK(conn1.connected());

    signal();
    CHECK(count0 == 1);
    CHECK(count1 == 1);

    conn0.disconnect();
    CHECK(!conn0.connected());
    CHECK( conn1.connected());

    signal();
    CHECK(count0 == 1);
    CHECK(count1 == 2);

    conn1.disconnect();
    CHECK(!conn0.connected());
    CHECK(!conn1.connected());

    signal();
    CHECK(count0 == 1);
    CHECK(count1 == 2);
  }

  SECTION("Scoped") {
    {
      auto conn = signal.connect([&]() { ++count0; }).scoped();
      signal();
      CHECK(count0 == 1);
    }

    signal();
    CHECK(count0 == 1);
  }
}

TEST_CASE("Signal destruction") {
  Connection<> connection;

  {
    Signal<> signal;
    connection = signal.connect([&](){});
    CHECK(connection.connected());
  }

  CHECK(!connection.connected());
}
