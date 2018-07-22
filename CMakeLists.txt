cmake_minimum_required(VERSION 3.9)

option(Build_tests "Requires GTest" OFF)

project(Functional)

add_subdirectory(functional)

if (Build_tests)
  add_subdirectory(test)
endif()
