cmake_minimum_required(VERSION 3.2)

project(Vortex2D)

option(ENABLE_EXAMPLES "Build examples" OFF)
option(ENABLE_TESTS "Build tests" OFF)
option(USE_OSMESA "Build using OSMesa instead of OpenGL for the tests/examples" OFF)

# cmake file to download and include dependencies
include(Dependencies/DownloadProject.cmake)
set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")

download_project(PROJ                glm
                 GIT_REPOSITORY      https://github.com/g-truc/glm.git
                 GIT_TAG             0.9.8.4
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})
add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})

add_subdirectory(Dependencies/glad)
add_subdirectory(Vortex2D)

if(ENABLE_EXAMPLES OR ENABLE_TESTS)
  if(USE_OSMESA)
    set(GLFW_USE_OSMESA ON)
    if (WIN32)
      set(OPENGL_LIB_NAME osmesa)
    else()
      set(OPENGL_LIB_NAME OSMesa)
    endif()
  else()
    find_package(OpenGL REQUIRED)
    set(OPENGL_LIB_NAME ${OPENGL_LIBRARIES})
  endif()
  download_project(PROJ                glfw3
                   GIT_REPOSITORY      https://github.com/glfw/glfw.git
                   GIT_TAG             master
                   ${UPDATE_DISCONNECTED_IF_AVAILABLE})
  set(GLFW_BUILD_EXAMPLES OFF)
  set(GLFW_BUILD_TESTS OFF)
  set(GLFW_BUILD_DOCS OFF)
  add_subdirectory(${glfw3_SOURCE_DIR} ${glfw3_BINARY_DIR})
  add_subdirectory(RenderWindow)
endif()

if (ENABLE_EXAMPLES)
  add_subdirectory(Examples)
endif()

if(ENABLE_TESTS)
  download_project(PROJ                googletest
                   GIT_REPOSITORY      https://github.com/google/googletest.git
                   GIT_TAG             release-1.8.0
                   ${UPDATE_DISCONNECTED_IF_AVAILABLE})

  # Prevent GoogleTest from overriding our compiler/linker options
  # when building with Visual Studio
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
  add_subdirectory(Tests)
endif()
