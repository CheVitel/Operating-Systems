cmake_minimum_required(VERSION 3.20)
project(Lab4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(lab4_common STATIC
    Lab4.cpp
    Lab4.h
)

add_executable(Receiver
    Receiver.cpp
)
target_link_libraries(Receiver PRIVATE lab4_common)

add_executable(Sender
    Sender.cpp
)
target_link_libraries(Sender PRIVATE lab4_common)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(Lab4Test
    tests/Lab4Test.cpp
)
target_link_libraries(Lab4Test PRIVATE lab4_common GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(Lab4Test)
