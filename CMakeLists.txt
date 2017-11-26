cmake_minimum_required(VERSION 3.2)

project(Vortex2D)

option(VORTEX2D_ENABLE_EXAMPLES "Build examples" OFF)
option(VORTEX2D_ENABLE_TESTS "Build tests" OFF)
option(VORTEX2D_ENABLE_PERFTESTS "Build performance tests" OFF)

# Only do coverage builds for gcc for the moment
if (CMAKE_COMPILER_IS_GNUCXX)
  include(Dependencies/CodeCoverage.cmake)
endif ()

# cmake file to download and include dependencies
include(Dependencies/DownloadProject.cmake)
set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")

download_project(PROJ                glm
                 GIT_REPOSITORY      https://github.com/g-truc/glm.git
                 GIT_TAG             0.9.8.4
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})
add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
add_subdirectory(Vortex2D)

if (VORTEX2D_ENABLE_EXAMPLES)
  add_subdirectory(VulkanGLFW)
  add_subdirectory(Examples)
endif ()

if (VORTEX2D_ENABLE_TESTS)
  enable_testing()
  download_project(PROJ                googletest
                   GIT_REPOSITORY      https://github.com/google/googletest.git
                   GIT_TAG             release-1.8.0
                   ${UPDATE_DISCONNECTED_IF_AVAILABLE})

  # Prevent GoogleTest from overriding our compiler/linker options
  # when building with Visual Studio
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
  add_subdirectory(Tests)
endif ()

if (VORTEX2D_ENABLE_PERFTESTS)
  enable_testing()
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  download_project(PROJ                benchmark
                   GIT_REPOSITORY      https://github.com/google/benchmark.git
                   GIT_TAG             v1.2.0
                   ${UPDATE_DISCONNECTED_IF_AVAILABLE})
  add_subdirectory(${benchmark_SOURCE_DIR} ${benchmark_BINARY_DIR})
  add_subdirectory(PerfTests)
endif ()
