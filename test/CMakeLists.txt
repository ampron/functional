cmake_minimum_required(VERSION 3.9)

project(FunctionalTest)

set(CMAKE_CXX_STANDARD 14)

add_executable(test)
add_executable(Functional::Test ALIAS test)

target_link_libraries(test PRIVATE Functional::Functional)

target_sources(test
  PRIVATE
  all_tests.cpp
  testing.h
)
