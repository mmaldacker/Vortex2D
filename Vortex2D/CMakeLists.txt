file(GLOB LIB_SOURCES
    "Vortex2D.h"
    "Engine/Density.h"
    "Engine/Density.cpp"
    "Engine/LevelSet.h"
    "Engine/LevelSet.cpp"
    "Engine/Pressure.h"
    "Engine/Pressure.cpp"
    "Engine/Advection.h"
    "Engine/Advection.cpp"
    "Engine/Extrapolation.h"
    "Engine/Extrapolation.cpp"
    "Engine/World.h"
    "Engine/World.cpp"
    "Engine/Size.h"
    "Engine/Size.cpp"
    "Engine/Boundaries.h"
    "Engine/Boundaries.cpp"
    "Engine/PrefixScan.h"
    "Engine/PrefixScan.cpp"
    "Engine/Particles.h"
    "Engine/Particles.cpp"
    "Engine/Rigidbody.h"
    "Engine/Rigidbody.cpp"
    "Engine/Velocity.h"
    "Engine/Velocity.cpp"
    "Engine/LinearSolver/LinearSolver.h"
    "Engine/LinearSolver/LinearSolver.cpp"
    "Engine/LinearSolver/Preconditioner.h"
    "Engine/LinearSolver/Reduce.h"
    "Engine/LinearSolver/Reduce.cpp"
    "Engine/LinearSolver/GaussSeidel.h"
    "Engine/LinearSolver/GaussSeidel.cpp"
    "Engine/LinearSolver/Jacobi.h"
    "Engine/LinearSolver/Jacobi.cpp"
    "Engine/LinearSolver/ConjugateGradient.h"
    "Engine/LinearSolver/ConjugateGradient.cpp"
    "Engine/LinearSolver/Diagonal.h"
    "Engine/LinearSolver/Diagonal.cpp"
    "Engine/LinearSolver/IncompletePoisson.h"
    "Engine/LinearSolver/IncompletePoisson.cpp"
    "Engine/LinearSolver/Transfer.h"
    "Engine/LinearSolver/Transfer.cpp"
    "Engine/LinearSolver/Multigrid.h"
    "Engine/LinearSolver/Multigrid.cpp"
    "Renderer/*.cpp"
    "Renderer/*.h"
    "SPIRV/Reflection.h"
    "SPIRV/Reflection.cpp"
    "Utils/variant.hpp"
    "Utils/vk_mem_alloc.h")

file(GLOB SHADER_SOURCES
    "Renderer/Kernels/*.vert"
    "Renderer/Kernels/*.frag"
    "Engine/Kernels/Advect.comp"
    "Engine/Kernels/AdvectVelocity.comp"
    "Engine/Kernels/BuildDiv.comp"
    "Engine/Kernels/BuildMatrix.comp"
    "Engine/Kernels/Extrapolate.comp"
    "Engine/Kernels/Project.comp"
    "Engine/Kernels/Redistance.comp"
    "Engine/Kernels/ConstrainVelocity.comp"
    "Engine/Kernels/ExtrapolateVelocity.comp"
    "Engine/Kernels/PolygonDist.frag"
    "Engine/Kernels/PolygonVelocity.frag"
    "Engine/Kernels/PolygonVelocity.vert"
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
    "Engine/Kernels/ParticleCloud.vert"
    "Engine/Kernels/ParticleCloud.frag"
    "Engine/Kernels/ParticlePhi.comp"
    "Engine/Kernels/ParticleToGrid.comp"
    "Engine/Kernels/ParticleFromGrid.comp"
    "Engine/Kernels/AdvectParticles.comp"
    "Engine/LinearSolver/Kernels/*.comp")

download_project(PROJ                SPIRV_Cross
                 GIT_REPOSITORY      https://github.com/KhronosGroup/SPIRV-Cross.git
                 GIT_TAG             master
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})
add_subdirectory(${SPIRV_Cross_SOURCE_DIR} ${SPIRV_Cross_BINARY_DIR})

download_project(PROJ                vkLoader
                 GIT_REPOSITORY      https://github.com/mmaldacker/vkLoader.git
                 GIT_TAG             master
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})
add_subdirectory(${vkLoader_SOURCE_DIR} ${vkLoader_BINARY_DIR})

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
       COMMAND python ${COMPILE_SCRIPT} --compiler ${GLSL_VALIDATOR} --output ${SHADER_OUTPUT} ${SHADER_SOURCES}
       DEPENDS ${SHADER_SOURCES} ${COMPILE_SCRIPT})

endfunction()

compile_shader(SOURCES ${SHADER_SOURCES} OUTPUT "vortex2d_generated_spirv")
add_library(vortex2d
  STATIC
    ${LIB_SOURCES}
    ${SHADER_SOURCES}
    "Engine/Kernels/CommonAdvect.comp"
    "Engine/Kernels/CommonProject.comp"
    "Engine/Kernels/CommonPreScan.comp"
    vortex2d_generated_spirv.cpp
    vortex2d_generated_spirv.h)

target_link_libraries(vortex2d PUBLIC glm spirv-cross-core vk_loader)
target_include_directories(vortex2d PUBLIC ../ ${CMAKE_CURRENT_BINARY_DIR})

target_compile_features(vortex2d PRIVATE cxx_variadic_templates cxx_generic_lambdas)
