file(GLOB SOURCES *.cpp)

include_directories(${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME})

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -march=native -O3)

if(BUILD_TESTING OR BUILD_BENCHMARKS)
    file(GLOB LIB_SOURCES *.cpp)

    list(REMOVE_ITEM LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Main.cpp")
    add_library(${PROJECT_NAME}lib STATIC ${LIB_SOURCES})
    target_compile_options(${PROJECT_NAME}lib PRIVATE -Wall -march=native)
endif()