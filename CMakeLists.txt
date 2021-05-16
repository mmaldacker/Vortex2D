cmake_minimum_required(VERSION 3.11)

project(Vortex2D)

set(CMAKE_CXX_STANDARD 14)

# setup version numbers
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

option(VORTEX2D_ENABLE_EXAMPLES "Build examples" OFF)
option(VORTEX2D_ENABLE_TESTS "Build tests" OFF)
option(VORTEX2D_ENABLE_DOCS "Build docs" OFF)

# Only do coverage builds for gcc for the moment
if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_COVERAGE
        "-g -O0 --coverage -fprofile-arcs -ftest-coverage"
        CACHE STRING "Flags used by the C++ compiler during coverage builds."
        FORCE )
    set(CMAKE_C_FLAGS_COVERAGE
        "-g -O0 --coverage -fprofile-arcs -ftest-coverage"
        CACHE STRING "Flags used by the C compiler during coverage builds."
        FORCE )
endif ()

include(cmake/Macros.cmake)
include(FetchContent)

set(GLM_INSTALL_ENABLE OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glm
                     GIT_REPOSITORY      https://github.com/g-truc/glm.git
                     GIT_TAG             0.9.9.0)

FetchContent_GetProperties(glm)
if(NOT glm_POPULATED)
    FetchContent_Populate(glm)
    add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
endif()

add_subdirectory(Vortex)

if (VORTEX2D_ENABLE_EXAMPLES)
  add_subdirectory(Examples)
endif ()

if (VORTEX2D_ENABLE_TESTS)
  add_subdirectory(Tests)
endif ()

if (VORTEX2D_ENABLE_DOCS)
  add_subdirectory(Docs)
endif ()
