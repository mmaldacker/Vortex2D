cmake_minimum_required(VERSION 3.2)

project(Vortex2D)

option(VORTEX2D_ENABLE_EXAMPLES "Build examples" OFF)
option(VORTEX2D_ENABLE_TESTS "Build tests" OFF)
option(VORTEX2D_ENABLE_PERFTESTS "Build performance tests" OFF)

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

# cmake file to download and include dependencies
include(cmake/DownloadProject.cmake)
set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")

if (VORTEX2D_ENABLE_TESTS OR VORTEX2D_ENABLE_PERFTESTS)
download_project(PROJ                FluidRigidCoupling2D
                 GIT_REPOSITORY      https://github.com/mmaldacker/FluidRigidCoupling2D.git
                 GIT_TAG             master
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})
add_subdirectory(${FluidRigidCoupling2D_SOURCE_DIR} ${FluidRigidCoupling2D_BINARY_DIR})
endif ()

add_subdirectory(Vortex2D)

if (VORTEX2D_ENABLE_EXAMPLES)
  add_subdirectory(Examples)
endif ()

if (VORTEX2D_ENABLE_TESTS)
  add_subdirectory(Tests)
endif ()

if (VORTEX2D_ENABLE_PERFTESTS)
  add_subdirectory(PerfTests)
endif ()
