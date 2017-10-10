

download_project(PROJ                Box2D
                 GIT_REPOSITORY      https://github.com/erincatto/Box2D.git
                 GIT_TAG             v2.3.1
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})

set(BOX2D_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${Box2D_SOURCE_DIR}/Box2D ${Box2D_BINARY_DIR})

file(GLOB EXAMPLES_SOURCES
        "main.cpp"
        "RenderExample.h"
        "ObstacleSmokeExample.h"
        "ScaleWaterExample.h"
        "SmokeExample.h"
        "WaterExample.h")

# Necessary since Box2D CMakeLists.txt doesn't use modern cmake
include_directories(${Box2D_SOURCE_DIR}/Box2D)

add_executable(vortex2d_examples ${EXAMPLES_SOURCES})
target_compile_features(vortex2d_examples PRIVATE cxx_nullptr)
target_link_libraries(vortex2d_examples vortex2d vortex2d_window Box2D)
