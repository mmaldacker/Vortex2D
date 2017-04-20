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
