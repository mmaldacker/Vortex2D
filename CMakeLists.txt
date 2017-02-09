cmake_minimum_required(VERSION 3.1)

project(Vortex2D)

option(ENABLE_EXAMPLES "Build examples" OFF)
option(ENABLE_TESTS "Build tests" OFF)
option(USE_OSMESA "Build using OSMesa instead of OpenGL for the tests/examples" OFF)

add_subdirectory(Vortex2D)

if (USE_OSMESA)
  if (WIN32)
    set (OPENGL_LIB_NAME osmesa)
  else ()
    set (OPENGL_LIB_NAME OSMesa)
  endif ()
else()
  find_package(OpenGL REQUIRED)
  set (OPENGL_LIB_NAME ${OPENGL_LIBRARIES})
endif()

if (ENABLE_EXAMPLES OR ENABLE_TESTS)
  add_subdirectory(GLFW)
endif()

if (ENABLE_EXAMPLES)
  add_subdirectory(Examples)
endif()

if (ENABLE_TESTS)
  add_subdirectory(Tests)
endif()
