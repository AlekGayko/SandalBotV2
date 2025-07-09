cmake_minimum_required(VERSION 3.22)

project(SandalBotV2 VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(BUILD_TESTING "Enable tests" OFF)
option(BUILD_BENCHMARKS "Enable benchmarks" OFF)

add_subdirectory(src)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()