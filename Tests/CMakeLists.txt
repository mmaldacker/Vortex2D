file(GLOB ENGINE_TEST_SOURCES "Engine/LevelSetTests.cpp"
                              "Engine/LinearSolverTests.cpp"
                              "Engine/ShaderTests.cpp"
                              "Engine/PressureTests.cpp"
                              "Engine/AdvectionTests.cpp"
                              "Engine/ExtrapolateTests.cpp"
                              "Engine/BoundariesTests.cpp"
                              "Engine/WorldTests.cpp"
                              "Engine/DensityTests.cpp"
                              "Engine/Helpers.h")

file(GLOB VARIATIONAL_SOURCES "variationalplusgfm/*.h"
                              "variationalplusgfm/*.cpp"
                              "variationalplusgfm/pcgsolver/*.h")

file(GLOB RENDERER_TEST_SOURCES  "Renderer/ShapeDrawer.h"
                                 "Renderer/ShapeDrawer.cpp"
                                 "Renderer/RenderingTests.cpp"
                                 "Renderer/ShapeTests.cpp"
                                 "Renderer/ComputeTests.cpp")

file(GLOB SHADER_TEST_SOURCES "Kernels/Image.comp"
                              "Kernels/Buffer.comp")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(vortex2d_tests ${RENDERER_TEST_SOURCES} ${VARIATIONAL_SOURCES} ${SHADER_TEST_SOURCES} "main.cpp" "Verify.h")
compile_shader(TARGET vortex2d_tests SOURCES ${SHADER_TEST_SOURCES})

target_compile_features(vortex2d_tests PRIVATE cxx_variadic_templates)
target_link_libraries(vortex2d_tests vortex2d gtest gmock gmock_main Threads::Threads)
target_include_directories(vortex2d_tests PRIVATE "./")

add_test(AllTests vortex2d_tests)

if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    setup_target_for_coverage(vortex2d_tests_coverage vortex2d_tests coverage)
endif ()
