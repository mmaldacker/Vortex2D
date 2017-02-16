if (VORTEX2D_USE_OSMESA)
  set(GLFW_USE_OSMESA ON CACHE BOOL "" FORCE)
else()
  find_package(OpenGL REQUIRED)
  include_directories(SYSTEM ${OPENGL_INCLUDE_DIR})
  set(VORTEX2D_LIBRARIES ${OPENGL_LIBRARIES})
endif ()

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
download_project(PROJ                glfw3
                 GIT_REPOSITORY      https://github.com/glfw/glfw.git
                 GIT_TAG             master
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})
add_subdirectory(${glfw3_SOURCE_DIR} ${glfw3_BINARY_DIR})

add_library(vortex2d_window STATIC
        "RenderWindow.h"
        "RenderWindow.cpp"
        "glfw.h"
        "glfw.cpp")

target_compile_features(vortex2d_window PRIVATE cxx_nullptr)
target_link_libraries(vortex2d_window PUBLIC vortex2d glfw ${VORTEX2D_LIBRARIES})
target_include_directories(vortex2d_window PUBLIC .)

if(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(CORE_VIDEO_FRAMEWORK CoreVideo)

    target_link_libraries(vortex2d_window PUBLIC ${COCOA_FRAMEWORK} ${IOKIT_FRAMEWORK} ${CORE_VIDEO_FRAMEWORK})
endif()

source_group(Examples REGULAR_EXPRESSION ".*(h|cpp)")
