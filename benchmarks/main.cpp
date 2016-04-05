#include <iomanip>
#include <iostream>
#include <chrono>
#include "secs/container.h"

// TODO: these benchmarks are too simplisitc be meanigful, improve them!

namespace chrono = std::chrono;

using std::cout;
using std::endl;
using std::make_unique;
using std::string;
using std::unique_ptr;
using std::vector;

using namespace secs;

struct Velocity {
  float x = 0;
  float y = 0;

  Velocity(float x = 0, float y = 0)
    : x(x), y(y)
  {}
};

static const size_t COUNT = 100000;

template<typename F>
void benchmark(const string& label, F&& body) {
  auto t0 = chrono::high_resolution_clock::now();
  body();
  auto t1 = chrono::high_resolution_clock::now();

  cout << std::left << std::setw(50) << label
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

static float sum = 0;

void use(float value) {
  sum += value;
}

////////////////////////////////////////////////////////////////////////////////
void iterate_vector_of_values() {
  vector<Velocity> inputs;
  inputs.reserve(COUNT);

  for (size_t i = 0; i < COUNT; ++i) {
    inputs.emplace_back(random_number(), random_number());
  }

  float result = 0;

  benchmark("iterate vector of values", [&]() {
    for (auto& input : inputs) {
      result += compute(input);
    }
  });

  use(result);
}

void iterate_vector_of_pointers() {
  vector<unique_ptr<Velocity>> inputs;
  inputs.reserve(COUNT);

  for (size_t i = 0; i < COUNT; ++i) {
    inputs.push_back(make_unique<Velocity>(random_number(), random_number()));
  }

  float result = 0;

  benchmark("iterate vector of pointers", [&]() {
    for (auto& input : inputs) {
      result += compute(*input);
    }
  });

  use(result);
}

void iterate_container() {
  Container container;

  for (size_t i = 0; i < COUNT; ++i) {
    auto e = container.create();
    e.create_component<Velocity>(random_number(), random_number());
  }

  float result = 0;

  benchmark("iterate container", [&]() {
    for (auto e : container.entities()) {
      result += compute(*e.component<Velocity>());
    }
  });

  use(result);
}

void iterate_container_with_required_components() {
  Container container;

  for (size_t i = 0; i < COUNT; ++i) {
    auto e = container.create();
    e.create_component<Velocity>(random_number(), random_number());
  }

  float result = 0;

  benchmark("iterate container with required components", [&]() {
    for (auto e : container.entities<Velocity>()) {
      result += compute(*e.component<Velocity>());
    }
  });

  use(result);
}

void iterate_container_with_optional_components() {
  Container container;

  for (size_t i = 0; i < COUNT; ++i) {
    auto e = container.create();
    e.create_component<Velocity>(random_number(), random_number());
  }

  float result = 0;

  benchmark("iterate container with optional components", [&]() {
    for (auto e : container.entities<Optional<Velocity>>()) {
      result += compute(*e.component<Velocity>());
    }
  });

  use(result);
}

void compare_component_ptr_and_raw_ptr() {
  Container container;

  for (size_t i = 0; i < COUNT; ++i) {
    auto e = container.create();
    e.create_component<Velocity>(random_number(), random_number());
  }

  std::vector<secs::ComponentPtr<Velocity>> component_ptrs;
  component_ptrs.reserve(COUNT);

  std::vector<Velocity*> raw_ptrs;
  raw_ptrs.reserve(COUNT);

  for (auto e : container.entities<Velocity>()) {
    component_ptrs.push_back(e.component<Velocity>());
    raw_ptrs.push_back(e.component<Velocity>().get());
  }

  float result = 0;

  benchmark("ComponentPtr<T> access", [&]() {
    for (auto p : component_ptrs) {
      result += compute(*p);
    }
  });

  benchmark("T* access", [&]() {
    for (auto p : raw_ptrs) {
      result += compute(*p);
    }
  });

  use(result);
}

int main() {
  iterate_vector_of_values();
  iterate_vector_of_pointers();
  iterate_container();
  iterate_container_with_required_components();
  iterate_container_with_optional_components();

  compare_component_ptr_and_raw_ptr();

  return 0;
}