cmake_minimum_required(VERSION 3.9)

project(Functional VERSION 0.5.0)

set(CMAKE_CXX_STANDARD 14)

# generate the version header file
configure_file("src/version.inline.h" "${PROJECT_BINARY_DIR}/version.h")

set(PUBLIC_HEADERS
    include/fun/option.h
    include/fun/option/option_inner.h
    include/fun/option/option.declare.h
    include/fun/option/option.impl.h
    include/fun/result.h
    include/fun/result/result.declare.h
    include/fun/result/result.impl.h
    include/fun/pipe.h
    include/fun/type_support.h
)

add_library(functional INTERFACE)
add_library(Functional::Functional ALIAS functional)

target_include_directories(functional INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# This hack ensures the sources files for this interface library show up in IDEs
# https://stackoverflow.com/a/29214327/1217063
add_custom_target(functional_ide_source SOURCES ${PUBLIC_HEADERS} "src/version.inline.h")
set_target_properties(functional_ide_source PROPERTIES
  PROJECT_LABEL "functional"
)

# generate the CMake confgure version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file("cmake/FunctionalConfigVersion.cmake"
  VERSION ${Functional_VERSION}
  COMPATIBILITY SameMinorVersion
)

install(
  EXPORT FunctionalTargets
  NAMESPACE Functional::
  DESTINATION lib/cmake/Functional
)

install(
  TARGETS functional
  EXPORT FunctionalTargets
)

# install public header directory as is
install(DIRECTORY include/fun DESTINATION include)

# install generated version header file
install(
  FILES "${PROJECT_BINARY_DIR}/version.h"
  DESTINATION include/fun
)

# install the CMake configure files so the user has easy "find_package" capabilities
install(
  FILES "cmake/FunctionalConfig.cmake" "${PROJECT_BINARY_DIR}/cmake/FunctionalConfigVersion.cmake"
  DESTINATION lib/cmake/Functional
)
