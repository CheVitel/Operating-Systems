cmake_minimum_required(VERSION 3.15)
project(Lab5Project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(receiver receiver.cpp Lab4Queue.cpp)
target_include_directories(receiver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(sender sender.cpp Lab4Queue.cpp)
target_include_directories(sender PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test_lab4 test/test_lab4.cpp Lab4Queue.cpp)
target_include_directories(test_lab4 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_lab4 PRIVATE GTest::gtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_lab4)
