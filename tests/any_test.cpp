#include "catch.hpp"
#include "secs/any.h"

// DEBUG
#include <iostream>

using namespace secs;

namespace {

struct Instrument {
  std::string data;

  std::function<void()> on_copy;
  std::function<void()> on_move;
  std::function<void()> on_destroy;

  Instrument(const std::string& data = {})
    : data(data)
  {}

  ~Instrument() {
    if (on_destroy) on_destroy();
  }

  Instrument(const Instrument& other)
    : data(other.data)
    , on_copy(other.on_copy)
    , on_move(other.on_move)
    , on_destroy(other.on_destroy)
  {
    if (on_copy) on_copy();
  }

  Instrument(Instrument&& other)
    : data(std::move(other.data))
    , on_copy(std::move(other.on_copy))
    , on_move(std::move(other.on_move))
    , on_destroy(std::move(other.on_destroy))
  {
    if (on_move) on_move();
  }
};

struct NonCopyable {
  NonCopyable() = default;
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable(NonCopyable&&) = default;
};

} // anonymous namespace

TEST_CASE("POD types") {
  Any a;
  CHECK(!a);
  CHECK(!a.contains<int>());

  a = 123456;
  CHECK(a);
  CHECK(a.contains<int>());
  CHECK(a.get<int>() == 123456);

  Any b = 654321;
  CHECK(b);
  CHECK(b.contains<int>());
  CHECK(b.get<int>() == 654321);

  Any c = std::move(b);
  CHECK(c.contains<int>());
  CHECK(c.get<int>() == 654321);
  CHECK(!b);
}

TEST_CASE("Non-POD types") {
  int copies   = 0;
  int moves    = 0;
  int destroys = 0;

  {
    Instrument i("hello world");
    i.on_copy    = [&]() { ++copies; };
    i.on_move    = [&]() { ++moves; };
    i.on_destroy = [&]() { ++destroys; };

    Any a = std::move(i);
    CHECK(a);
    CHECK(a.contains<Instrument>());
    CHECK(a.get<Instrument>().data == "hello world");

    CHECK(copies   == 0);
    CHECK(moves    == 1);
    CHECK(destroys == 0);

    Any b = std::move(a);
    CHECK(copies   == 0);
    CHECK(moves    == 1);
    CHECK(destroys == 0);
    CHECK(b.get<Instrument>().data == "hello world");

    Any c;
    c = std::move(b);
    CHECK(copies   == 0);
    CHECK(moves    == 1);
    CHECK(destroys == 0);
    CHECK(c.get<Instrument>().data == "hello world");

    // Move moved value
    Any d = std::move(a);
    CHECK(copies   == 0);
    CHECK(moves    == 1);
    CHECK(destroys == 0);
    CHECK(!d);

    c.reset();
    CHECK(destroys == 1);
  }

  CHECK(destroys == 1);
}

TEST_CASE("Non-copyable types") {
  Any a = NonCopyable{};
  Any b = std::move(a);

  std::vector<Any> many;
  many.push_back(NonCopyable{});
  many.resize(1024);
}
