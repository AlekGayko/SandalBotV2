file(GLOB BENCH_SRC_FILES *.cpp)

include_directories(${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME})

add_executable(Benchmarks ${BENCH_SRC_FILES})
target_link_libraries(Benchmarks PRIVATE yourlib benchmark::benchmark)