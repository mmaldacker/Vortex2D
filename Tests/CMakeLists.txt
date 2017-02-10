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

enable_testing()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(vortex2d_tests ${TEST_SOURCES})
target_compile_features(vortex2d_tests PRIVATE cxx_variadic_templates)
target_link_libraries(vortex2d_tests vortex2d vortex2d_window gtest gmock gmock_main Threads::Threads)
target_include_directories(vortex2d_tests PRIVATE "variationalplusgfm")

add_test(AllTests vortex2d_tests)

source_group(Tests REGULAR_EXPRESSION ".*(h|cpp)")
source_group(variationalplusgfm REGULAR_EXPRESSION "variationalplusgfm/.*(h|cpp)")
source_group(variationalplusgfm\\pcgsolver REGULAR_EXPRESSION "variationalplusgfm/pcgsolver/.*(h|cpp)")
