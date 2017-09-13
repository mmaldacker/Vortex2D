file(GLOB ENGINE_TEST_SOURCES "Engine/LevelSetTests.cpp"
                              "Engine/LinearSolverTests.cpp"
                              "Engine/PressureTests.cpp"
                              "Engine/AdvectionTests.cpp"
                              "Engine/ExtrapolateTests.cpp"
                              "Engine/BoundariesTests.cpp")

file(GLOB VARIATIONAL_SOURCES "Engine/VariationalHelpers.h"
                              "variationalplusgfm/*.h"
                              "variationalplusgfm/*.cpp"
                              "variationalplusgfm/pcgsolver/*.h")

file(GLOB RENDERER_TEST_SOURCES  "Renderer/ShapeDrawer.h"
                                 "Renderer/ShapeDrawer.cpp"
                                 "Renderer/RenderingTests.cpp"
                                 "Renderer/ShapeTests.cpp"
                                 "Renderer/ComputeTests.cpp")

file(GLOB SHADER_TEST_SOURCES "Kernels/Image.comp"
                              "Kernels/Buffer.comp"
                              "Kernels/Work.comp"
                              "Kernels/Stencil.comp"
                              "Kernels/Checkerboard.comp")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(vortex2d_tests_lib STATIC  ${VARIATIONAL_SOURCES})
target_compile_features(vortex2d_tests_lib PRIVATE cxx_variadic_templates)
target_link_libraries(vortex2d_tests_lib vortex2d gtest gmock gmock_main Threads::Threads)
target_include_directories(vortex2d_tests_lib PUBLIC "./" "./variationalplusgfm")

add_executable(vortex2d_tests ${ENGINE_TEST_SOURCES} ${RENDERER_TEST_SOURCES} ${SHADER_TEST_SOURCES} "main.cpp" "Verify.h")
target_compile_features(vortex2d_tests PRIVATE cxx_variadic_templates)
target_link_libraries(vortex2d_tests vortex2d_tests_lib)

compile_shader(TARGET vortex2d_tests SOURCES ${SHADER_TEST_SOURCES})
copy_shader(TARGET vortex2d_tests SOURCES vortex2d)

add_test(AllTests vortex2d_tests)

if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    setup_target_for_coverage(vortex2d_tests_coverage vortex2d_tests coverage)
endif ()
