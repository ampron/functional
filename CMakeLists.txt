cmake_minimum_required(VERSION 3.9)

option(Build_tests "Requires GTest" OFF)

project(Functional)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_subdirectory(functional)

if (Build_tests)
  add_subdirectory(test)
endif()
