
file(GLOB LIB_SOURCES "Engine/*.cpp" 
					  "Engine/*.h" 
					  "Engine/LinearSolver/*.h"
					  "Engine/LinearSolver/*.cpp"
					  "Renderer/*.cpp" 
					  "Renderer/*.h")
file(GLOB EXAMPLES_SOURCES "Examples/*.h" "Examples/*.cpp")

find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)

add_library(vortex2d STATIC ${LIB_SOURCES})
target_include_directories(vortex2d PUBLIC Engine Engine/LinearSolver Renderer)
target_link_libraries(vortex2d ${OPENGL_LIBRARIES})

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

add_executable(vortex2d_examples ${EXAMPLES_SOURCES})

find_library(COCOA_FRAMEWORK Cocoa)
find_library(IOKIT_FRAMEWORK IOKit)
find_library(CORE_VIDEO_FRAMEWORK CoreVideo)

target_link_libraries(vortex2d_examples ${COCOA_FRAMEWORK} ${IOKIT_FRAMEWORK} ${CORE_VIDEO_FRAMEWORK})

target_link_libraries(vortex2d_examples vortex2d ${GLFW_STATIC_LIBRARIES})