file(GLOB TEST_SRC_FILES *.cpp)

include(
    ${PROJECT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
)
include(GoogleTest)
include_directories(
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}    
    ${PROJECT_SOURCE_DIR}/build/src
)

add_executable(UnitTests ${TEST_SRC_FILES})

gtest_discover_tests(UnitTests)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(UnitTests 
    PRIVATE 
        ${PROJECT_NAME}lib 
        GTest::gtest 
        GTest::gtest_main
)

target_compile_definitions(UnitTests PRIVATE TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/tests/")

add_test(NAME AllTests COMMAND UnitTests)