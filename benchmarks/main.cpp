#include <iomanip>
#include <iostream>
#include <chrono>
#include "secs/container.h"

namespace chrono = std::chrono;

using std::cout;
using std::endl;
using std::make_unique;
using std::string;
using std::unique_ptr;
using std::vector;

using secs::Container;

struct Velocity {
  float x = 0;
  float y = 0;

  Velocity(float x = 0, float y = 0)
    : x(x), y(y)
  {}
};

static const size_t COUNT = 10000;

template<typename F>
void benchmark(const string& label, F&& body) {
  auto t0 = chrono::high_resolution_clock::now();
  body();
  auto t1 = chrono::high_resolution_clock::now();

  cout << std::left << std::setw(40) << label
       << ": "
       << std::right << std::setw(8)
       << chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count()
       << " ns\n";
}

static std::default_random_engine gen;

float random_number() {
  std::uniform_real_distribution<float> dist(-1, 1);
  return dist(gen);
}

float compute(Velocity& v) {
  return v.x + v.y;
}

////////////////////////////////////////////////////////////////////////////////
void vector_of_values() {
  vector<Velocity> inputs;
  inputs.reserve(COUNT);

  for (size_t i = 0; i < COUNT; ++i) {
    inputs.emplace_back(random_number(), random_number());
  }

  vector<float> outputs;
  outputs.reserve(COUNT);

  benchmark("vector of values", [&]() {
    for (auto& input : inputs) {
      outputs.push_back(compute(input));
    }
  });
}

void vector_of_pointers() {
  vector<unique_ptr<Velocity>> inputs;
  inputs.reserve(COUNT);

  for (size_t i = 0; i < COUNT; ++i) {
    inputs.push_back(make_unique<Velocity>(random_number(), random_number()));
  }

  vector<float> outputs;
  outputs.reserve(COUNT);

  benchmark("vector of pointers", [&]() {
    for (auto& input : inputs) {
      outputs.push_back(compute(*input));
    }
  });
}

void container_with_entity() {
  Container container;

  for (size_t i = 0; i < COUNT; ++i) {
    auto e = container.create();
    e.create_component<Velocity>(random_number(), random_number());
  }

  vector<float> outputs;
  outputs.reserve(COUNT);

  benchmark("container iteration via entity", [&]() {
    for (auto c : container.entities().need<Velocity>()) {
      outputs.push_back(compute(*c.entity().component<Velocity>()));
    }
  });
}

void container_with_cursor() {
  Container container;

  for (size_t i = 0; i < COUNT; ++i) {
    auto e = container.create();
    e.create_component<Velocity>(random_number(), random_number());
  }

  vector<float> outputs;
  outputs.reserve(COUNT);

  benchmark("container iteration via cursor", [&]() {
    for (auto c : container.entities().need<Velocity>()) {
      outputs.push_back(compute(c.get<Velocity>()));
    }
  });
}

int main() {
  vector_of_values();
  vector_of_pointers();
  container_with_entity();
  container_with_cursor();

  return 0;
}