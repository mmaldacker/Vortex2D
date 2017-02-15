file(GLOB EXAMPLES_SOURCES
        "main.cpp"
        "ObstacleSmokeExample.h"
        "ScaleWaterExample.h"
        "SmokeExample.h"
        "WaterExample.h")

add_executable(vortex2d_examples ${EXAMPLES_SOURCES})
target_compile_features(vortex2d_examples PRIVATE cxx_nullptr)
target_link_libraries(vortex2d_examples vortex2d vortex2d_window)

source_group(Examples REGULAR_EXPRESSION ".*(h|cpp)")
