file(GLOB EXAMPLES_SOURCES
        "main.cpp"
        "BaseExample.h"
        "ObstacleSmokeExample.h"
        "ScaleWaterExample.h"
        "SmokeExample.h"
        "WaterExample.h"
        "VelocitySmokeExample")

if (USE_OSMESA_FOR_TESTS)
  if (WIN32)
    set (OPENGL_LIB_NAME osmesa)
  else ()
    set (OPENGL_LIB_NAME OSMesa)
  endif ()
else()
  find_package(OpenGL REQUIRED)
  set (OPENGL_LIB_NAME ${OPENGL_LIBRARIES})
endif()

add_executable(vortex2d_examples ${EXAMPLES_SOURCES})
target_compile_features(vortex2d_examples PRIVATE cxx_nullptr)
target_link_libraries(vortex2d_examples ${OPENGL_LIB_NAME} vortex2d_window)

source_group(Examples REGULAR_EXPRESSION ".*(h|cpp)")
