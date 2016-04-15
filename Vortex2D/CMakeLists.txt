
file(GLOB LIB_SOURCES "Engine/*.cpp" 
					  "Engine/*.h" 
					  "Engine/LinearSolver/*.h"
					  "Engine/LinearSolver/*.cpp"
					  "Renderer/*.cpp" 
					  "Renderer/*.h")

find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)

add_library(vortex2d STATIC ${LIB_SOURCES})
target_include_directories(vortex2d PUBLIC Engine Engine/LinearSolver Renderer)
target_link_libraries(vortex2d ${OPENGL_LIBRARIES})