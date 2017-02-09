file(GLOB TEST_SOURCES "LevelSetTests.cpp"
                       "RenderingTests.cpp"
                       "LinearSolverTests.cpp"
                       "ShaderTests.cpp"
                       "PressureTests.cpp"
                       "AdvectionTests.cpp"
                       "ExtrapolateTests.cpp"
                       "BoundariesTests.cpp"
                       "Helpers.h"
                       "main.cpp"
                       "variationalplusgfm/*.h"
                       "variationalplusgfm/*.cpp"
                       "variationalplusgfm/pcgsolver/*.h")

find_package(glfw3 REQUIRED)

enable_testing()

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

find_library(GMOCK_LIBRARY NAMES gmock PATHS ${GTEST_ROOT} PATH_SUFFIXES lib)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if (WIN32)
  set (OSMESA_LIB_NAME osmesa)
else ()
  set (OSMESA_LIB_NAME OSMesa)
endif ()

add_executable(vortex2d_tests ${TEST_SOURCES})
target_compile_features(vortex2d_tests PRIVATE cxx_variadic_templates)
target_link_libraries(vortex2d_tests ${OSMESA_LIB_NAME} glfw vortex2d vortex2d_window ${GTEST_BOTH_LIBRARIES} ${GMOCK_LIBRARY} Threads::Threads)
target_include_directories(vortex2d_tests PRIVATE "variationalplusgfm")

add_test(AllTests vortex2d_tests)

source_group(Tests REGULAR_EXPRESSION ".*(h|cpp)")
source_group(variationalplusgfm REGULAR_EXPRESSION "variationalplusgfm/.*(h|cpp)")
source_group(variationalplusgfm\\pcgsolver REGULAR_EXPRESSION "variationalplusgfm/pcgsolver/.*(h|cpp)")
