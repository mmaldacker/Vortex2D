set(LIB_SOURCES
    "Engine/Density.cpp"
    "Engine/LevelSet.cpp"
    "Engine/Pressure.cpp"
    "Engine/Advection.cpp"
    "Engine/Extrapolation.cpp"
    "Engine/World.cpp"
    "Engine/Size.cpp"
    "Engine/Boundaries.cpp"
    "Engine/PrefixScan.cpp"
    "Engine/Particles.cpp"
    "Engine/Rigidbody.cpp"
    "Engine/Velocity.cpp"
    "Engine/LinearSolver/LinearSolver.cpp"
    "Engine/LinearSolver/Reduce.cpp"
    "Engine/LinearSolver/GaussSeidel.cpp"
    "Engine/LinearSolver/Jacobi.cpp"
    "Engine/LinearSolver/ConjugateGradient.cpp"
    "Engine/LinearSolver/Diagonal.cpp"
    "Engine/LinearSolver/IncompletePoisson.cpp"
    "Engine/LinearSolver/Transfer.cpp"
    "Engine/LinearSolver/Multigrid.cpp"
    "Renderer/Buffer.cpp"
    "Renderer/CommandBuffer.cpp"
    "Renderer/DescriptorSet.cpp"
    "Renderer/Device.cpp"
    "Renderer/Instance.cpp"
    "Renderer/Pipeline.cpp"
    "Renderer/RenderState.cpp"
    "Renderer/RenderTexture.cpp"
    "Renderer/RenderWindow.cpp"
    "Renderer/RenderTarget.cpp"
    "Renderer/Shapes.cpp"
    "Renderer/Sprite.cpp"
    "Renderer/Texture.cpp"
    "Renderer/Timer.cpp"
    "Renderer/Transformable.cpp"
    "Renderer/Work.cpp"
    "SPIRV/Reflection.cpp")

set(LIB_HEADERS
    "Vortex2D.h"
    "Engine/Density.h"
    "Engine/LevelSet.h"
    "Engine/Pressure.h"
    "Engine/Advection.h"
    "Engine/Extrapolation.h"
    "Engine/World.h"
    "Engine/Size.h"
    "Engine/Boundaries.h"
    "Engine/PrefixScan.h"
    "Engine/Particles.h"
    "Engine/Rigidbody.h"
    "Engine/Velocity.h"
    "Engine/LinearSolver/LinearSolver.h"
    "Engine/LinearSolver/Preconditioner.h"
    "Engine/LinearSolver/Reduce.h"
    "Engine/LinearSolver/GaussSeidel.h"
    "Engine/LinearSolver/Jacobi.h"
    "Engine/LinearSolver/ConjugateGradient.h"
    "Engine/LinearSolver/Diagonal.h"
    "Engine/LinearSolver/IncompletePoisson.h"
    "Engine/LinearSolver/Transfer.h"
    "Engine/LinearSolver/Multigrid.h"
    "Renderer/Common.h"
    "Renderer/Drawable.h"
    "Renderer/Buffer.h"
    "Renderer/CommandBuffer.h"
    "Renderer/DescriptorSet.h"
    "Renderer/Device.h"
    "Renderer/Instance.h"
    "Renderer/Pipeline.h"
    "Renderer/RenderState.h"
    "Renderer/RenderTexture.h"
    "Renderer/RenderWindow.h"
    "Renderer/RenderTarget.h"
    "Renderer/Shapes.h"
    "Renderer/Sprite.h"
    "Renderer/Texture.h"
    "Renderer/Timer.h"
    "Renderer/Transformable.h"
    "Renderer/Work.h"
    "SPIRV/Reflection.h"
    "Utils/variant.hpp"
    "Utils/vk_mem_alloc.h")

file(GLOB SHADER_SOURCES
    "Renderer/Kernels/*.vert"
    "Renderer/Kernels/*.frag"
    "Engine/Kernels/Advect.comp"
    "Engine/Kernels/AdvectVelocity.comp"
    "Engine/Kernels/BuildDiv.comp"
    "Engine/Kernels/BuildRigidbodyDiv.comp"
    "Engine/Kernels/BuildMatrix.comp"
    "Engine/Kernels/Extrapolate.comp"
    "Engine/Kernels/Project.comp"
    "Engine/Kernels/RigidbodyPressure.comp"
    "Engine/Kernels/Redistance.comp"
    "Engine/Kernels/ConstrainVelocity.comp"
    "Engine/Kernels/ConstrainRigidbodyVelocity.comp"
    "Engine/Kernels/ExtrapolateVelocity.comp"
    "Engine/Kernels/PolygonDist.frag"
    "Engine/Kernels/CircleDist.frag"
    "Engine/Kernels/UpdateVertices.comp"
    "Engine/Kernels/DistanceField.frag"
    "Engine/Kernels/PreScanAdd.comp"
    "Engine/Kernels/PreScan.comp"
    "Engine/Kernels/PreScanStoreSum.comp"
    "Engine/Kernels/ParticleCount.comp"
    "Engine/Kernels/ParticleClamp.comp"
    "Engine/Kernels/ParticleSpawn.comp"
    "Engine/Kernels/ParticleBucket.comp"
    "Engine/Kernels/ParticlePhi.comp"
    "Engine/Kernels/ParticleToGrid.comp"
    "Engine/Kernels/ParticleFromGrid.comp"
    "Engine/Kernels/AdvectParticles.comp"
    "Engine/Kernels/VelocityDifference.comp"
    "Engine/LinearSolver/Kernels/*.comp")

download_project(PROJ                glm
                 GIT_REPOSITORY      https://github.com/g-truc/glm.git
                 GIT_TAG             0.9.8.4
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})
add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})

download_project(PROJ                SPIRV_Cross
                 GIT_REPOSITORY      https://github.com/KhronosGroup/SPIRV-Cross.git
                 GIT_TAG             master
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})
add_subdirectory(${SPIRV_Cross_SOURCE_DIR} ${SPIRV_Cross_BINARY_DIR})
set_property(TARGET spirv-cross-core PROPERTY POSITION_INDEPENDENT_CODE ON)

find_package(PythonInterp REQUIRED)
find_package(Vulkan REQUIRED)

function(compile_shader)
    set(options "")
    set(oneValueArgs OUTPUT)
    set(multiValueArgs SOURCES)

    cmake_parse_arguments(SHADER "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    find_program(GLSL_VALIDATOR glslc hints "$ENV{VULKAN_SDK}/Bin")
    message("Using compiler: ${GLSL_VALIDATOR}")
    
    set(COMPILE_SCRIPT ${CMAKE_SOURCE_DIR}/Scripts/GenerateSPIRV.py)
    add_custom_command(
       OUTPUT "${SHADER_OUTPUT}.h" "${SHADER_OUTPUT}.cpp"
       COMMAND ${PYTHON_EXECUTABLE} ${COMPILE_SCRIPT} --compiler ${GLSL_VALIDATOR} --output ${SHADER_OUTPUT} ${SHADER_SOURCES}
       DEPENDS ${SHADER_SOURCES} ${COMPILE_SCRIPT})

endfunction()

compile_shader(SOURCES ${SHADER_SOURCES} OUTPUT "vortex2d_generated_spirv")

add_library(vortex2d
  SHARED
    ${LIB_SOURCES}
    ${LIB_HEADERS}
    ${SHADER_SOURCES}
    "Engine/Kernels/CommonAdvect.comp"
    "Engine/Kernels/CommonProject.comp"
    "Engine/Kernels/CommonPreScan.comp"
    "Engine/Kernels/CommonParticles.comp"
    vortex2d_generated_spirv.cpp
    vortex2d_generated_spirv.h)

target_link_libraries(vortex2d PUBLIC glm Vulkan::Vulkan PRIVATE spirv-cross-core)

target_include_directories(vortex2d
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/Vortex2D>
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_compile_features(vortex2d PRIVATE cxx_variadic_templates cxx_generic_lambdas)

if(WIN32)
    target_compile_definitions(vortex2d PRIVATE VORTEX2D_API_EXPORTS)
endif()

if(MSVC)
    target_compile_options(vortex2d PUBLIC "/W3")
else()
    target_compile_options(vortex2d PUBLIC "-Wall")
endif()

install(TARGETS vortex2d
        EXPORT Vortex2DTargets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(DIRECTORY . DESTINATION include/Vortex2D
        FILES_MATCHING PATTERN "*.h")

configure_file("${CMAKE_SOURCE_DIR}/cmake/Vortex2DConfig.cmake.in" "${PROJECT_BINARY_DIR}/Vortex2DConfig.cmake" @ONLY)

if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/CMake/Vortex2D)
endif()

set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

install(FILES "${PROJECT_BINARY_DIR}/Vortex2DConfig.cmake"
        DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

install(EXPORT Vortex2DTargets DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
