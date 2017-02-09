cmake_minimum_required(VERSION 3.1)

project(Vortex2D)

option(ENABLE_EXAMPLES "Build examples" OFF)
option(ENABLE_TESTS "Build tests" OFF)
option(USE_OSMESA_FOR_TESTS "Build using OSMesa instead of OpenGL for the tests" OFF)

add_subdirectory(Vortex2D)

if (ENABLE_EXAMPLES OR ENABLE_TESTS)
  add_subdirectory(GLFW)
endif()

if (ENABLE_EXAMPLES)
  add_subdirectory(Examples)
endif()

if (ENABLE_TESTS)
  add_subdirectory(Tests)
endif()
