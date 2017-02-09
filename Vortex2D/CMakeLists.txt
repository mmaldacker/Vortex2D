file(GLOB LIB_SOURCES
        "Engine/*.cpp"
        "Engine/*.h"
        "Engine/LinearSolver/*.h"
        "Engine/LinearSolver/*.cpp"
        "Renderer/*.cpp"
        "Renderer/*.h")

find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_LIBRARIES})

add_library(vortex2d STATIC ${LIB_SOURCES})
target_include_directories(vortex2d PUBLIC Engine Engine/LinearSolver Renderer)

target_compile_features(vortex2d PRIVATE cxx_variadic_templates)

source_group(Renderer REGULAR_EXPRESSION "Renderer/.*(h|cpp)")
source_group(Engine REGULAR_EXPRESSION "Engine/.*(h|cpp)")
source_group(Engine\\LinearSolver REGULAR_EXPRESSION "Engine/LinearSolver/.*(h|cpp)")
