cmake_minimum_required(VERSION 3.11)

project(FunctionalTest)

set(CMAKE_CXX_STANDARD 14)

find_package(GTest REQUIRED)

add_executable(test)
add_executable(Functional::Test ALIAS test)

target_link_libraries(test PRIVATE Functional::Functional GTest::GTest)

target_sources(test
  PRIVATE
  all_tests.cpp
  testing.h
)
