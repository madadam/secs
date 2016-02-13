#include <chrono>
#include <iostream>

#include "environment.h"

namespace chrono = std::chrono;
using std::cout;
using std::endl;
using std::string;

using secs::Environment;
using secs::System;

template<typename T>
void unused(T) {}

template<typename F>
chrono::microseconds benchmark(F block) {
  auto start = chrono::high_resolution_clock::now();
  block();
  auto end = chrono::high_resolution_clock::now();

  return chrono::duration_cast<chrono::microseconds>(end - start);
}

string to_string(chrono::microseconds us) {
  return std::to_string(((float) us.count()) / 1000.0);
}

struct Name {
  string name;
};

struct Position {
  float x = 0;
  float y = 0;
};

struct Shape {
  float radius = 0;
};

struct Motion {
  float dx = 0;
  float dy = 0;
};

struct DebugSystem : public System<> {
  void update(Environment<>& env) override {
    env.entities<Name, Position>().each([](auto&, auto& name, auto& position) {
      cout << "name: " << name.name
           << " position: (" << position.x << ", " << position.y << ")"
           << endl;
    });
  }
};

int main(int /*argc*/, char** /*argv*/) {
  Environment<> env;

  auto a = env.create_entity();
  auto b = env.create_entity();

  a.add_component(Name{ "Alice" });
  a.add_component(Position{ 0, 0 });
  a.add_component(Shape{ 10 });

  b.add_component(Name{ "Bob" });
  b.add_component(Position{ 10, 0 });
  b.add_component(Motion{ -1, 0 });
  b.add_component(Shape{ 2 });

  env.emplace_system<DebugSystem>();
  env.update();

  // a.destroy();
  // a.add_component(Motion{ 0, 0 });

  return 0;
}
