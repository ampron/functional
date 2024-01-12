cmake_minimum_required(VERSION 3.11)

project(FunctionalTest)

set(CMAKE_CXX_STANDARD 17)

cmake_policy(SET CMP0135 NEW)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(test)
add_executable(Functional::Test ALIAS test)

target_link_libraries(test PRIVATE Functional::Functional GTest::gtest)

target_sources(test
  PRIVATE
  all_tests.cpp
  testing.h
)

include(GoogleTest)
gtest_discover_tests(test)
