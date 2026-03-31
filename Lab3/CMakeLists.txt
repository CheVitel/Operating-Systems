cmake_minimum_required(VERSION 3.15)
project(Lab3Project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(Lab3 Lab3.cpp)
target_include_directories(Lab3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test_lab3 test/test_lab3.cpp Lab3.cpp)
target_include_directories(test_lab3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(test_lab3 PRIVATE BUILDING_FOR_TESTS)
target_link_libraries(test_lab3 PRIVATE GTest::gtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_lab3)