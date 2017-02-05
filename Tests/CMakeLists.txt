file(GLOB TEST_SOURCES "LevelSetTests.cpp"
                       "RenderingTests.cpp"
                       "LinearSolverTests.cpp"
                       "ShaderTests.cpp"
                       "PressureTests.cpp"
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

add_executable(vortex2d_tests ${TEST_SOURCES})
target_compile_features(vortex2d_tests PRIVATE cxx_variadic_templates)
target_link_libraries(vortex2d_tests vortex2d glfw vortex2d_window ${GTEST_BOTH_LIBRARIES} ${GMOCK_LIBRARY})
target_include_directories(vortex2d_tests PRIVATE "variationalplusgfm")

add_test(AllTests vortex2d_tests)

source_group(Tests REGULAR_EXPRESSION ".*(h|cpp)")
source_group(variationalplusgfm REGULAR_EXPRESSION "variationalplusgfm/.*(h|cpp)")
source_group(variationalplusgfm\\pcgsolver REGULAR_EXPRESSION "variationalplusgfm/pcgsolver/.*(h|cpp)")
