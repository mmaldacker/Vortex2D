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
FetchContent_Declare(googletest
                     GIT_REPOSITORY      https://github.com/google/googletest.git
                     GIT_TAG             master)

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Disable install
set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

FetchContent_Declare(fluidrigidcoupling2d
                     GIT_REPOSITORY      https://github.com/mmaldacker/FluidRigidCoupling2D.git
                     GIT_TAG             master)

FetchContent_GetProperties(fluidrigidcoupling2d)
if(NOT fluidrigidcoupling2d_POPULATED)
    FetchContent_Populate(fluidrigidcoupling2d)
    add_subdirectory(${fluidrigidcoupling2d_SOURCE_DIR} ${fluidrigidcoupling2d_BINARY_DIR})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

compile_shader(SOURCES ${SHADER_TEST_SOURCES} OUTPUT "vortex2d_tests_generated_spirv" VERSION 1.0)

set(SOURCE_FILES
  ${ENGINE_TEST_SOURCES}
  ${RENDERER_TEST_SOURCES}
  ${SHADER_TEST_SOURCES}
  "main.cpp" "Verify.h"
  ${CMAKE_CURRENT_BINARY_DIR}/vortex2d_tests_generated_spirv.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/vortex2d_tests_generated_spirv.h)

set(CMAKE_DIR "${PROJECT_SOURCE_DIR}/cmake")

# create a bundle for unit-tests on ios so they can run on it.
if (IOS_PLATFORM)
  add_executable(vortex2d_tests MACOSX_BUNDLE ${SOURCE_FILES})
  set_target_properties(vortex2d_tests PROPERTIES
                        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${CODE_SIGN_IDENTITY}
                        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${DEVELOPMENT_TEAM_ID}
                        MACOSX_BUNDLE_GUI_IDENTIFIER org.vortex2d.tests
                        XCODE_ATTRIBUTE_ENABLE_TESTABILITY YES
                        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/Frameworks"
                        )
  copy_vortex2d_framework(vortex2d_tests)
else()
  add_executable(vortex2d_tests ${SOURCE_FILES})
endif()

target_link_libraries(vortex2d_tests vortex2d gtest gmock gmock_main Threads::Threads fluidrigidcoupling2d_lib)
target_include_directories(vortex2d_tests PRIVATE ./ ${CMAKE_CURRENT_BINARY_DIR})

if (WIN32)
    vortex2d_copy_dll(vortex2d_tests)
endif()

add_test(AllTests vortex2d_tests)
