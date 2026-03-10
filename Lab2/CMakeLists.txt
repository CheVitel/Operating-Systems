cmake_minimum_required(VERSION 3.14)
project(Lab2Project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_library(Lab2Lib STATIC
  Lab2.cpp
)
target_compile_definitions(Lab2Lib PRIVATE BUILDING_FOR_TESTS)
target_include_directories(Lab2Lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Lab2
  Lab2.cpp
  Lab2.h
)

add_executable(Lab2Tests
  test.cpp
)
target_link_libraries(Lab2Tests
  PRIVATE
    Lab2Lib
    GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(Lab2Tests)