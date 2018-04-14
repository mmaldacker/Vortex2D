file(GLOB ENGINE_TEST_SOURCES "Engine/LevelSetTests.cpp"
                              "Engine/LinearSolverTests.cpp"
                              "Engine/PressureTests.cpp"
                              "Engine/AdvectionTests.cpp"
                              "Engine/ExtrapolateTests.cpp"
                              "Engine/BoundariesTests.cpp"
                              "Engine/ParticleTests.cpp"
                              "Engine/WorldTests.cpp"
                              "Engine/RigidbodyTests.cpp"
                              "Engine/VariationalHelpers.h"
                              "Engine/VariationalHelpers.cpp")

file(GLOB RENDERER_TEST_SOURCES  "Renderer/ShapeDrawer.h"
                                 "Renderer/ShapeDrawer.cpp"
                                 "Renderer/RenderingTests.cpp"
                                 "Renderer/ShapeTests.cpp"
                                 "Renderer/ComputeTests.cpp")

file(GLOB SHADER_TEST_SOURCES "Kernels/Image.comp"
                              "Kernels/ImageFloat.comp"
                              "Kernels/Buffer.comp"
                              "Kernels/Work.comp"
                              "Kernels/WorkIndirect.comp"
                              "Kernels/Stencil.comp"
                              "Kernels/Checkerboard.comp")

enable_testing()
download_project(PROJ                googletest
               GIT_REPOSITORY      https://github.com/google/googletest.git
               GIT_TAG             master
               ${UPDATE_DISCONNECTED_IF_AVAILABLE})

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Disable install
set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

download_project(PROJ                FluidRigidCoupling2D
                 GIT_REPOSITORY      https://github.com/mmaldacker/FluidRigidCoupling2D.git
                 GIT_TAG             master
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})
add_subdirectory(${FluidRigidCoupling2D_SOURCE_DIR} ${FluidRigidCoupling2D_BINARY_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

compile_shader(SOURCES ${SHADER_TEST_SOURCES} OUTPUT "vortex2d_tests_generated_spirv")

add_executable(vortex2d_tests ${ENGINE_TEST_SOURCES} ${RENDERER_TEST_SOURCES} ${SHADER_TEST_SOURCES} "main.cpp" "Verify.h" ${CMAKE_CURRENT_BINARY_DIR}/vortex2d_tests_generated_spirv.cpp ${CMAKE_CURRENT_BINARY_DIR}/vortex2d_tests_generated_spirv.h)
target_compile_features(vortex2d_tests PRIVATE cxx_variadic_templates cxx_generic_lambdas)
target_link_libraries(vortex2d_tests vortex2d gtest gmock gmock_main Threads::Threads fluidrigidcoupling2d_lib)
target_include_directories(vortex2d_tests PRIVATE ./ ${CMAKE_CURRENT_BINARY_DIR})

if (WIN32)
    add_custom_command(
        TARGET vortex2d_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_BINARY_DIR}/Vortex2D/vortex2d.dll"
            $<TARGET_FILE_DIR:vortex2d_tests>)
endif()

add_test(AllTests vortex2d_tests)
