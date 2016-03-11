cmake_minimum_required (VERSION 3.2)

set(DEFAULT_CXX_FLAGS "-std=c++14 -Wall -Wextra -pedantic -Werror")
include_directories("${CMAKE_SOURCE_DIR}/include")

#-------------------------------------------------------------------------------
project(lib)

set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         ${DEFAULT_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${DEFAULT_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${DEFAULT_CXX_FLAGS}")

file(GLOB sources "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_library(secs ${sources})

#-------------------------------------------------------------------------------
project(tests)

set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         ${DEFAULT_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${DEFAULT_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${DEFAULT_CXX_FLAGS}")

file(GLOB sources "${CMAKE_SOURCE_DIR}/tests/*.cpp")

add_executable(tests ${sources})
target_link_libraries(tests secs)

#-------------------------------------------------------------------------------
project(benchmarks)

set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         ${DEFAULT_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${DEFAULT_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${DEFAULT_CXX_FLAGS}")

file(GLOB sources "${CMAKE_SOURCE_DIR}/benchmarks/*.cpp")

add_executable(benchmarks ${sources})
target_link_libraries(benchmarks secs)
